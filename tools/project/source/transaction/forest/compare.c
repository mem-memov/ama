/** @file
 * @brief Recognize exact material changes independently of ordering and timestamps.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * preserves bytes and identities; a graph or modification time cannot replace them.
 */
#include <string.h>
#include "internal.h"
const SourceCatalogImageEntry *source_forest_find(const SourceCatalogImage *image, const char *path) {
    for (size_t i = 0; i < image->count; i++) { if (!strcmp(image->entries[i].path, path)) { return &image->entries[i]; } }
    return NULL;
}
bool source_forest_equal(const SourceCatalogImageEntry *a, const SourceCatalogImageEntry *b) {
    if (!a || !b) { return a == b; }
    if (a->metadata_captured != b->metadata_captured) { return false; }
    if (a->metadata_captured && (a->metadata.st_dev != b->metadata.st_dev || a->metadata.st_ino != b->metadata.st_ino
        || a->metadata.st_mode != b->metadata.st_mode)) { return false; }
    if (!a->content || !b->content) { return a->content == b->content; }
    return source_length(a->content) == source_length(b->content)
        && !memcmp(source_bytes(a->content), source_bytes(b->content), source_length(a->content));
}
