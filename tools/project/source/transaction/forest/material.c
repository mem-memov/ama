/** @file
 * @brief Retain complete byte-sorted before/after material for each disjoint mutation tree.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * cannot stage or recover unread bytes or incomplete directory membership.
 */
#include <stdint.h>
#include <string.h>
#include "internal.h"
SourceForestStatus source_forest_material(SourceArena *arena, const SourceCatalogImage *image, const char *path,
    const SourceCatalogImageEntry *const **output, size_t *size, SourceForestIssue *issue) {
    size_t count = 0;
    for (size_t i = 0; i < image->count; i++) {
        const SourceCatalogImageEntry *row = &image->entries[i];
        if (!source_image_below(row->path, path)) { continue; }
        issue->path = row->path; issue->entry = row;
        if (!row->metadata_captured || (S_ISDIR(row->metadata.st_mode) ? !row->children_complete : S_ISREG(row->metadata.st_mode) && !row->content)) { return SOURCE_FOREST_UNOBSERVED; }
        if (!S_ISDIR(row->metadata.st_mode) && !S_ISREG(row->metadata.st_mode)) { return SOURCE_FOREST_UNSUPPORTED; }
        count++;
    }
    if (count > SIZE_MAX / sizeof(SourceCatalogImageEntry *)) { return SOURCE_FOREST_LIMIT; }
    const SourceCatalogImageEntry **rows = count ? source_arena_allocate(arena, count * sizeof(*rows)) : NULL;
    if (count && !rows) { return SOURCE_FOREST_MEMORY; }
    size_t used = 0;
    for (size_t i = 0; i < image->count; i++) {
        if (!source_image_below(image->entries[i].path, path)) { continue; }
        size_t index = used++;
        while (index && strcmp(rows[index - 1]->path, image->entries[i].path) > 0) { rows[index] = rows[index - 1]; index--; }
        rows[index] = &image->entries[i];
    }
    *output = rows; *size = count; return SOURCE_FOREST_OK;
}
