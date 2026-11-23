/** @file
 * @brief Prove stable complete parent authority for a mutation subtree's publication name.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires exact original membership and recoverable same-filesystem staging.
 */
#include <stdint.h>
#include <string.h>
#include "internal.h"
SourceForestStatus source_forest_parent(SourceArena *arena, const SourceCatalogImage *before,
    const SourceCatalogImage *after, SourceMutationRoot *root, SourceForestIssue *issue) {
    const char *slash = strrchr(root->path, '/');
    if (!slash || !slash[1]) { issue->path = root->path; return SOURCE_FOREST_PARENT; }
    size_t prefix = 0; while (root->path[prefix] == '/') { prefix++; }
    size_t length = (size_t)(slash - root->path); if (length < prefix) { length = prefix; }
    if (length == SIZE_MAX) { return SOURCE_FOREST_LIMIT; }
    char *parent = source_arena_allocate(arena, length + 1);
    if (!parent) { return SOURCE_FOREST_MEMORY; }
    memcpy(parent, root->path, length); parent[length] = 0;
    root->parent = parent; root->name = slash + 1;
    const SourceCatalogImageEntry *old = source_forest_find(before, parent), *now = source_forest_find(after, parent);
    issue->path = parent; issue->entry = old;
    if (!old || !old->metadata_captured || !S_ISDIR(old->metadata.st_mode) || !old->children_complete
        || !now || !source_forest_equal(old, now)) { return SOURCE_FOREST_PARENT; }
    root->parent_entry = old; return SOURCE_FOREST_OK;
}
