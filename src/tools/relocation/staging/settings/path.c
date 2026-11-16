/** @file
 * @brief Resolve explicit settings paths through verified existing directories and exact child spelling.
 * [TOOL-01](../../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * accepts documented explicit settings, without guessing namespaces or following aliases.
 */
#include <string.h>
#include "../internal.h"
#include "../../internal.h"
#include "source/path/exact.h"
/** The directory resolver owns temporary scratch; only the copied full path escapes into staging storage. Component contract: [TOOL-01](../../../../../spec/implementation/tools-and-conformance.md#tool-01). */
const char *relocation_staging_setting_path(RelocationStaging *staging, const char *input, bool missing, bool *existed) {
    if (input == NULL || *input == 0) {
        relocation_staging_fail(staging,RELOCATION_INVALID_PATH,"An explicit settings path is empty",(SourceSpan){0});
        return NULL;
    }
    char *copy = source_arena_copy(&staging->arena,input);
    if (copy == NULL) {
        relocation_staging_fail(staging,RELOCATION_OUT_OF_MEMORY,"Settings path allocation failed",(SourceSpan){0});
        return NULL;
    }
    char *slash = strrchr(copy,'/');
    const char *name = slash == NULL ? copy : slash+1;
    if (*name == 0 || strcmp(name,".") == 0 || strcmp(name,"..") == 0) {
        relocation_staging_fail(staging,RELOCATION_INVALID_PATH,"Settings paths require an explicit final entry",(SourceSpan){0});
        return NULL;
    }
    const char *parent = ".";
    if (slash != NULL) { *slash = 0; parent = slash == copy ? "/" : copy; }
    RelocationPreview authority;
    relocation_preview_init(&authority,staging->arena.allocator);
    const char *directory = relocation_path(&authority,parent,false);
    const char *full = directory == NULL ? NULL : source_arena_join(&staging->arena,directory,
        strcmp(directory,"/") == 0 ? "" : "/",name);
    RelocationStatus status = authority.status;
    relocation_preview_destroy(&authority);
    if (full == NULL) {
        relocation_staging_fail(staging,status == RELOCATION_OK ? RELOCATION_OUT_OF_MEMORY : status,
            "The explicit settings parent could not be resolved",(SourceSpan){0});
        return NULL;
    }
    char *verified_parent = source_arena_copy(&staging->arena,full);
    if (verified_parent == NULL) {
        relocation_staging_fail(staging,RELOCATION_OUT_OF_MEMORY,"Settings parent allocation failed",(SourceSpan){0});
        return NULL;
    }
    slash = strrchr(verified_parent,'/');
    name = full+(slash-verified_parent)+1;
    if (slash == verified_parent) { slash[1] = 0; } else { *slash = 0; }
    struct stat metadata;
    PathProbe probe = path_exact_child(verified_parent,name,full,&metadata);
    if (probe != PATH_FOUND && !(missing && probe == PATH_MISSING)) {
        relocation_staging_fail(staging,probe == PATH_IO_ERROR ? RELOCATION_IO_ERROR
            : probe == PATH_UNSUPPORTED ? RELOCATION_UNSUPPORTED_PATH : RELOCATION_INVALID_PATH,
            "The explicit settings entry could not be resolved exactly",(SourceSpan){0});
        return NULL;
    }
    *existed = probe == PATH_FOUND;
    return full;
}
