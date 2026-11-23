/** @file
 * @brief Select observed directory permissions for initialization under explicit final roots.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * separates semantic roots from physical evidence; absent-root scaffolding uses
 * the closest explicit physical boundary without introducing host reads.
 */
#include "internal.h"
#include <string.h>
/** Match immutable exact physical keys; no source discovery or normalization. */
static const SourceCatalogImageEntry *find(const SourceCatalogImage *image, const char *path) {
    for (size_t i = 0; i < image->count; i++) {
        if (!strcmp(image->entries[i].path, path)) { return &image->entries[i]; }
    }
    return NULL;
}
const SourceCatalogImageEntry *source_configuration_root_mode(const SourceCatalogImage *image, const ModuleRoot *root) {
    const SourceCatalogImageEntry *row = find(image, root->path);
    if (!row) {
        const char *closest = NULL;
        for (size_t i = 0; i < source_image_bound_count(image); i++) {
            const char *bound = source_image_bound_path(image, i);
            if (source_image_below(root->path, bound) && (!closest || strlen(bound) > strlen(closest))) { closest = bound; }
        }
        row = closest ? find(image, closest) : NULL;
    }
    return row && row->metadata_captured && S_ISDIR(row->metadata.st_mode) ? row : NULL;
}
