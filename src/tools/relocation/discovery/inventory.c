/** @file
 * @brief Capture the selected physical tree, retaining accompanying files without parsing them.
 * [TOOL-01](../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * moves relative layout intact and forbids silently extending S to sibling files.
 */
#include <string.h>
#include "../internal.h"

/** Append a complete observed entry; earlier directory identities prevent alias cycles before queueing. */
static bool append(RelocationPreview *preview, const char *path) {
    struct stat metadata;
    if (lstat(path,&metadata) != 0) { return relocation_fail(preview,RELOCATION_IO_ERROR,path,NULL); }
    if (!S_ISDIR(metadata.st_mode) && !S_ISREG(metadata.st_mode) && !S_ISLNK(metadata.st_mode)) {
        return relocation_fail(preview,RELOCATION_UNSUPPORTED_PATH,path,NULL);
    }
    if (S_ISDIR(metadata.st_mode)) {
        for (const RelocationEntry *other = preview->entries; other != NULL; other = other->next) {
            if (S_ISDIR(other->metadata.st_mode) && metadata.st_dev == other->metadata.st_dev
                && metadata.st_ino == other->metadata.st_ino) {
                return relocation_fail(preview,RELOCATION_ALIAS,path,other->before);
            }
        }
    }
    const char *after = relocation_project(preview,path);
    if (after == NULL) { return false; }
    RelocationEntry *entry = source_arena_allocate(&preview->arena,sizeof(*entry));
    if (entry == NULL) { return relocation_fail(preview,RELOCATION_OUT_OF_MEMORY,path,NULL); }
    *entry = (RelocationEntry){.before=path,.after=after,.metadata=metadata};
    if (preview->last_entry == NULL) { preview->entries = entry; }
    else { preview->last_entry->next = entry; }
    preview->last_entry = entry;
    return true;
}

/** Breadth-first queue entries own stable paths; only actual directories are opened, never symbolic links. */
bool relocation_inventory(RelocationPreview *preview) {
    if (!append(preview,preview->source)) { return false; }
    for (const RelocationEntry *entry = preview->entries; entry != NULL; entry = entry->next) {
        if (!S_ISDIR(entry->metadata.st_mode)) { continue; }
        const char **names = NULL;
        size_t count = 0;
        if (!relocation_entries(preview,entry->before,&names,&count)) { return false; }
        for (size_t index = 0; index < count; index++) {
            const char *path = source_arena_join(&preview->arena,entry->before,"/",names[index]);
            if (path == NULL) { return relocation_fail(preview,RELOCATION_OUT_OF_MEMORY,entry->before,NULL); }
            if (!append(preview,path)) { return false; }
        }
    }
    return true;
}
