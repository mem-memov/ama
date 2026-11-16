/** @file
 * @brief Queue final directories only after exact physical identity validation.
 * [SRC-02](../../../../spec/language/source-and-names.md#src-02) excludes aliases
 * even between empty roots; [ARCH-05](../../../../spec/implementation/compiler.md#arch-05)
 * requires a finite explicit traversal instead of recursion or guessed cutoffs.
 */
#include "internal.h"

/** Each admitted directory adds one previously unseen physical identity and one stable queue record. Component contract: [SRC-02](../../../../spec/language/source-and-names.md#src-02), [ARCH-05](../../../../spec/implementation/compiler.md#arch-05). */
bool relocation_queue(RelocationWalk *walk, const char *path, const char *relative, const char *logical) {
    RelocationPreview *preview = walk->preview;
    struct stat metadata;
    const char *physical = NULL;
    PathProbe probe = relocation_probe(preview,path,&physical,&metadata);
    if (probe != PATH_FOUND || !S_ISDIR(metadata.st_mode)) {
        return relocation_fail(preview,probe == PATH_UNSUPPORTED ? RELOCATION_UNSUPPORTED_PATH
            : probe == PATH_IO_ERROR ? RELOCATION_IO_ERROR : RELOCATION_INVALID_PATH,path,NULL);
    }
    for (const RelocationDirectoryIdentity *other = *walk->identities; other != NULL; other = other->next) {
        if (other->device == metadata.st_dev && other->inode == metadata.st_ino) {
            return relocation_fail(preview,RELOCATION_ALIAS,path,other->path);
        }
    }
    RelocationDirectoryIdentity *identity = source_arena_allocate(&preview->arena,sizeof(*identity));
    RelocationDirectory *directory = source_arena_allocate(&preview->arena,sizeof(*directory));
    if (identity == NULL || directory == NULL) { return relocation_fail(preview,RELOCATION_OUT_OF_MEMORY,path,NULL); }
    *identity = (RelocationDirectoryIdentity){*walk->identities,path,metadata.st_dev,metadata.st_ino};
    *walk->identities = identity;
    *directory = (RelocationDirectory){NULL,path,relative,logical};
    if (walk->last == NULL) { walk->pending = directory; }
    else { walk->last->next = directory; }
    walk->last = directory;
    return true;
}

/** The FIFO closes each finite directory exactly once; views read only original storage. Component contract: [SRC-02](../../../../spec/language/source-and-names.md#src-02), [ARCH-05](../../../../spec/implementation/compiler.md#arch-05). */
bool relocation_walk_root(RelocationPreview *preview, const ModuleRoot *root, RelocationDirectoryIdentity **identities) {
    RelocationWalk walk = {.preview=preview,.root=root,.identities=identities};
    if (!relocation_queue(&walk,root->path,"",root->kind == MODULE_ROOT_STANDARD ? "." : "")) { return false; }
    while (walk.pending != NULL) {
        RelocationDirectory *directory = walk.pending;
        walk.pending = directory->next;
        if (walk.pending == NULL) { walk.last = NULL; }
        const char **names = NULL;
        size_t count = 0;
        if (!relocation_view_entries(preview,directory->path,&names,&count)) { return false; }
        for (size_t index = 0; index < count; index++) {
            if (!relocation_visit(&walk,directory,names[index])) { return false; }
        }
    }
    return true;
}
