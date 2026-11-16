/** @file
 * @brief Verify complete published replacement bytes and directory membership before withdrawing them.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * does not discard user edits merely because an inode still matches the prepared object.
 */
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include "../internal.h"

/** A finite literal link is copied with checked growth and never followed, including missing targets and cycles. Component contract: [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
static bool link_text(RelocationRecovery *recovery, const char *path, const SourceText **output) {
    RelocationTransaction *transaction = &recovery->transaction;
    SourceAllocator allocator = recovery->sources.allocator;
    if (!relocation_transaction_permit(transaction,RELOCATION_IO_READ,path)) { return false; }
    for (size_t capacity = 64;;) {
        if (capacity > (size_t)SSIZE_MAX) {
            return relocation_transaction_fail(transaction,RELOCATION_OUT_OF_MEMORY,"A retained link exceeds the supported read range",path);
        }
        unsigned char *buffer = allocator.allocate(allocator.context,capacity);
        if (buffer == NULL) { return relocation_transaction_fail(transaction,RELOCATION_OUT_OF_MEMORY,"Retained link verification allocation failed",path); }
        ssize_t length = readlink(path,(char *)buffer,capacity);
        bool complete = length >= 0 && (size_t)length < capacity;
        SourceStatus status = complete ? source_manager_add(&recovery->sources,path,buffer,(size_t)length,output) : SOURCE_OK;
        allocator.release(allocator.context,buffer);
        if (length < 0) { return relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"A retained literal link could not be read",path); }
        if (status != SOURCE_OK) { return relocation_transaction_fail(transaction,RELOCATION_OUT_OF_MEMORY,"Retained link ownership allocation failed",path); }
        if (complete) { return true; }
        if (capacity > SIZE_MAX/2) { return relocation_transaction_fail(transaction,RELOCATION_OUT_OF_MEMORY,"A retained link size is unrepresentable",path); }
        capacity *= 2;
    }
}
/** Exact bytes are read only from retained immutable originals when no changed replacement payload was necessary. Component contract: [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
static bool expected(RelocationRecovery *recovery, const RelocationPreparedEntry *entry, const SourceText **output) {
    if (entry->file->replacement != NULL) { *output = entry->file->replacement; return true; }
    RelocationTransaction *transaction = &recovery->transaction;
    const RelocationTransactionAction *source = transaction->first;
    const char *base = relocation_transaction_probe(transaction,source->slot->backup,&source->original_metadata) == RELOCATION_PATH_EXPECTED
        ? source->slot->backup : source->path;
    const char *path = source_arena_join(&transaction->arena,base,"",entry->file->before+strlen(transaction->report.source));
    if (path == NULL) { return relocation_transaction_fail(transaction,RELOCATION_OUT_OF_MEMORY,"Retained original verification path allocation failed",NULL); }
    if (relocation_transaction_probe(transaction,path,&entry->file->metadata) != RELOCATION_PATH_EXPECTED) {
        return relocation_transaction_fail(transaction,RELOCATION_CHANGED_INPUT,"The retained original entry has changed identity",path);
    }
    if (S_ISLNK(entry->file->metadata.st_mode)) { return link_text(recovery,path,output); }
    if (!relocation_transaction_permit(transaction,RELOCATION_IO_READ,path)) { return false; }
    SourceStatus status = source_manager_load(&recovery->sources,path,output);
    return status == SOURCE_OK || relocation_transaction_fail(transaction,status == SOURCE_OUT_OF_MEMORY ? RELOCATION_OUT_OF_MEMORY : RELOCATION_IO_ERROR,
        "The retained original entry could not be read",path);
}
/** Compare exact regular/link bytes, with one close attempt per opened regular descriptor. Component contract: [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
static bool bytes(RelocationRecovery *recovery, const char *path, mode_t mode, const SourceText *value) {
    RelocationTransaction *transaction = &recovery->transaction;
    if (S_ISLNK(mode)) {
        const SourceText *actual;
        if (!link_text(recovery,path,&actual)) { return false; }
        return (source_length(actual) == source_length(value)
            && memcmp(source_bytes(actual),source_bytes(value),source_length(value)) == 0)
            || relocation_transaction_fail(transaction,RELOCATION_CHANGED_INPUT,"A published literal link has changed bytes",path);
    }
    int descriptor = open(path,O_RDONLY|O_NOFOLLOW|O_CLOEXEC);
    if (descriptor < 0) { return relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"A published replacement could not be opened",path); }
    bool complete = relocation_transaction_verify(transaction,descriptor,path,value);
    bool closed = relocation_transaction_close(transaction,descriptor,path);
    return complete && closed;
}
/** Every actual immediate child must occur in the complete flat tree; symlinks are entries, never traversal roots. Component contract: [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
static bool children(RelocationRecovery *recovery, const RelocationTransactionAction *action, const char *path) {
    RelocationTransaction *transaction = &recovery->transaction;
    DIR *directory = opendir(path);
    if (directory == NULL) { return relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"A published directory could not be enumerated",path); }
    bool complete = true;
    for (;;) {
        errno = 0;
        struct dirent *child = readdir(directory);
        if (child == NULL) {
            if (errno != 0) { complete = relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"Published directory enumeration failed",path); }
            break;
        }
        if (strcmp(child->d_name,".") == 0 || strcmp(child->d_name,"..") == 0) { continue; }
        bool known = false;
        size_t parent_length = strlen(path);
        for (size_t index = 1; index < action->tree_count; index++) {
            const char *candidate = action->tree[index].file->after;
            const char *slash = strrchr(candidate,'/');
            if ((size_t)(slash-candidate) == parent_length && strncmp(candidate,path,parent_length) == 0
                && strcmp(slash+1,child->d_name) == 0) { known = true; break; }
        }
        if (!known) { complete = relocation_transaction_fail(transaction,RELOCATION_CHANGED_INPUT,"A published replacement contains an unrecorded entry",path); break; }
    }
    if (closedir(directory) != 0) { complete = relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"A published directory could not be closed",path); }
    return complete;
}
/** Full inventory verification precedes any inverse, so failure retains both original and modified published state. Component contract: [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_recovery_replacement(RelocationRecovery *recovery, const RelocationTransactionAction *action) {
    if (action->kind == RELOCATION_ACTION_REPLACE) {
        return bytes(recovery,action->path,action->prepared_metadata.st_mode,action->file->replacement);
    }
    RelocationTransaction *transaction = &recovery->transaction;
    for (size_t index = 0; index < action->tree_count; index++) {
        const RelocationPreparedEntry *entry = &action->tree[index];
        const char *path = entry->file->after;
        struct stat actual;
        if (relocation_transaction_probe(transaction,path,&entry->metadata) != RELOCATION_PATH_EXPECTED
            || lstat(path,&actual) != 0 || actual.st_mode != entry->metadata.st_mode
            || actual.st_uid != entry->metadata.st_uid || actual.st_gid != entry->metadata.st_gid) {
            return relocation_transaction_fail(transaction,RELOCATION_CHANGED_INPUT,"A published replacement entry has changed identity or attributes",path);
        }
        if (S_ISDIR(actual.st_mode)) {
            if (!children(recovery,action,path)) { return false; }
        } else {
            const SourceText *value;
            if (!expected(recovery,entry,&value) || !bytes(recovery,path,actual.st_mode,value)) { return false; }
        }
    }
    return true;
}
