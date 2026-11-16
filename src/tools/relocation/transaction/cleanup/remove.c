/** @file
 * @brief Retire proven-owned private trees with an explicit descriptor stack and no symlink traversal.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * retains recovery data when cleanup cannot complete instead of reporting success.
 */
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "../internal.h"

/** Each open parent remains owned while its child is visited; paths are retained for diagnostics only. Component contract: [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
typedef struct RemovalFrame {
    struct RemovalFrame *parent;
    DIR *directory;
    const char *path;
    const char *name;
    struct stat metadata;
    size_t removals;
} RemovalFrame;

/** Close before checkpointing so a simulated close error cannot lose a live descriptor. Component contract: [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
static bool close_frame(RelocationTransaction *transaction, RemovalFrame *frame) {
    if (frame->directory == NULL) { return true; }
    int result = closedir(frame->directory);
    frame->directory = NULL;
    bool allowed = relocation_transaction_permit(transaction,RELOCATION_IO_CLOSE,frame->path);
    return (result == 0 && allowed)
        || relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"A private cleanup directory could not be closed",frame->path);
}
/** Open through the held parent descriptor; the final component cannot redirect traversal through a link. Component contract: [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
static RemovalFrame *push(RelocationTransaction *transaction, RemovalFrame *parent,
    const char *path, const char *name, const struct stat *expected) {
    RemovalFrame *frame = source_arena_allocate(&transaction->arena,sizeof(*frame));
    if (frame == NULL) {
        relocation_transaction_fail(transaction,RELOCATION_OUT_OF_MEMORY,"Private cleanup traversal allocation failed",path);
        return NULL;
    }
    *frame = (RemovalFrame){.parent=parent,.path=path,.name=name,.metadata=*expected};
    int parent_descriptor = parent == NULL ? AT_FDCWD : dirfd(parent->directory);
    int descriptor = openat(parent_descriptor,name,O_RDONLY|O_DIRECTORY|O_NOFOLLOW|O_CLOEXEC);
    if (descriptor < 0) {
        relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"A private cleanup directory could not be opened",path);
        return NULL;
    }
    struct stat actual;
    bool valid = fstat(descriptor,&actual) == 0 && actual.st_dev == expected->st_dev && actual.st_ino == expected->st_ino;
    if (valid && faccessat(descriptor,".",R_OK|W_OK|X_OK,0) != 0) {
        valid = relocation_transaction_permit(transaction,RELOCATION_IO_MODE,path)
            && fchmod(descriptor,(actual.st_mode&07777)|S_IRWXU) == 0;
    }
    if (!valid) {
        (void)relocation_transaction_close(transaction,descriptor,path);
        relocation_transaction_fail(transaction,RELOCATION_CHANGED_INPUT,"A private cleanup directory identity or permission changed",path);
        return NULL;
    }
    frame->directory = fdopendir(descriptor);
    if (frame->directory == NULL) {
        (void)relocation_transaction_close(transaction,descriptor,path);
        relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"A private cleanup directory could not be enumerated",path);
        return NULL;
    }
    return frame;
}

/** Every successful child deletion decreases the finite owned tree; failed prefixes retain their remaining bytes and close all frames. Component contract: [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_transaction_remove(RelocationTransaction *transaction, const char *path) {
    struct stat metadata;
    if (lstat(path,&metadata) != 0) {
        return errno == ENOENT || relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"A private cleanup path could not be inspected",path);
    }
    if (!S_ISDIR(metadata.st_mode)) {
        return relocation_transaction_permit(transaction,RELOCATION_IO_REMOVE,path)
            && (unlink(path) == 0 || relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"A private file could not be removed",path));
    }
    RemovalFrame *frame = push(transaction,NULL,path,path,&metadata);
    bool complete = frame != NULL;
    while (complete && frame != NULL) {
        errno = 0;
        struct dirent *entry = readdir(frame->directory);
        if (entry != NULL) {
            if (strcmp(entry->d_name,".") == 0 || strcmp(entry->d_name,"..") == 0) { continue; }
            const char *child = source_arena_join(&transaction->arena,frame->path,"/",entry->d_name);
            if (child == NULL) { complete = relocation_transaction_fail(transaction,RELOCATION_OUT_OF_MEMORY,"Private cleanup child path allocation failed",frame->path); break; }
            const char *name = strrchr(child,'/')+1;
            if (fstatat(dirfd(frame->directory),name,&metadata,AT_SYMLINK_NOFOLLOW) != 0) {
                complete = relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"A private cleanup child could not be inspected",child); break;
            }
            if (S_ISDIR(metadata.st_mode)) {
                RemovalFrame *next = push(transaction,frame,child,name,&metadata);
                if (next == NULL) { complete = false; break; }
                frame = next;
            } else if (!relocation_transaction_permit(transaction,RELOCATION_IO_REMOVE,child)
                || unlinkat(dirfd(frame->directory),name,0) != 0) {
                complete = relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"A private cleanup child could not be removed",child);
            } else { frame->removals++; }
            continue;
        }
        if (errno != 0) { complete = relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"A private cleanup directory read failed",frame->path); break; }
        RemovalFrame *parent = frame->parent;
        int parent_descriptor = parent == NULL ? AT_FDCWD : dirfd(parent->directory);
        if (!relocation_transaction_permit(transaction,RELOCATION_IO_REMOVE,frame->path)) { complete = false; break; }
        if (unlinkat(parent_descriptor,frame->name,AT_REMOVEDIR) != 0) {
            if (errno == ENOTEMPTY && frame->removals != 0) {
                /* Own deletions may invalidate enumeration offsets; observed remaining
                 * entries plus strict prior progress justify a fresh pass, not a guessed retry. */
                rewinddir(frame->directory);
                frame->removals = 0;
                continue;
            }
            complete = relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"A private cleanup directory could not be removed",frame->path); break;
        }
        if (parent != NULL) { parent->removals++; }
        if (!close_frame(transaction,frame)) { complete = false; break; }
        frame = parent;
    }
    while (frame != NULL) {
        if (!close_frame(transaction,frame)) { complete = false; }
        frame = frame->parent;
    }
    return complete;
}
