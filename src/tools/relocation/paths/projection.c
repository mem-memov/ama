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
/** Outside paths keep their original spelling; mapped paths retain their entire relative suffix. */
const char *relocation_project(RelocationPreview *preview, const char *path) {
    if (!relocation_contains(preview->source,path)) { return path; }
    const char *suffix = path+strlen(preview->source);
    const char *projected = source_arena_join(&preview->arena,preview->destination,"",suffix);
    if (projected == NULL) { relocation_fail(preview,RELOCATION_OUT_OF_MEMORY,path,NULL); }
    return projected;
}
