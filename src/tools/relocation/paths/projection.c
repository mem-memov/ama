/** @file
 * @brief Project physical placements by exact slash-delimited subtree membership.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * defines P independently of logical namespace names and structural plus markers.
 */
#include <string.h>
#include "../internal.h"
/** Normalized absolute inputs make root containment and sibling-prefix exclusion explicit. */
bool relocation_contains(const char *parent, const char *path) {
    size_t length = strlen(parent);
    return strcmp(parent,"/") == 0 || (strncmp(parent,path,length) == 0
        && (path[length] == 0 || path[length] == '/'));
}
/** Project into the caller's arena, borrowing the input spelling for unchanged outside paths. */
const char *relocation_project_to(SourceArena *arena, const RelocationPreview *preview, const char *path) {
    if (!relocation_contains(preview->source,path)) { return path; }
    return source_arena_join(arena,preview->destination,"",path+strlen(preview->source));
}
/** Discovery retains its own projected paths and records allocation failure in its own diagnostic owner. */
const char *relocation_project(RelocationPreview *preview, const char *path) {
    const char *projected = relocation_project_to(&preview->arena,preview,path);
    if (projected == NULL) { relocation_fail(preview,RELOCATION_OUT_OF_MEMORY,path,NULL); }
    return projected;
}
