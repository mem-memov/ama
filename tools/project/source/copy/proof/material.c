/** @file
 * @brief Compare prepared mapped material with exact final opaque bytes and directory observations.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * retains all copied input material; [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires evidence for the actual staged image rather than intended writes.
 */
#include <string.h>
#include "internal.h"
/** Compare actual output observations, including opaque files not in the catalog. */
bool source_copy_proof_material(const SourceCopyMaterial *material, const SourceCatalogImage *image, const char **path) {
    for (size_t i = 0; i < material->count; i++) {
        const SourceCopyMaterialEntry *row = &material->entries[i];
        const SourceCatalogImageEntry *found = NULL; *path = row->mapping->destination;
        for (size_t j = 0; j < image->count; j++) {
            if (!strcmp(image->entries[j].path, *path)) { found = &image->entries[j]; break; }
        }
        bool directory = row->mapping->original->directory;
        if (!found || !found->metadata_captured || (directory ? !S_ISDIR(found->metadata.st_mode) || !found->children_complete
            : !S_ISREG(found->metadata.st_mode) || !found->content || source_length(found->content) != source_length(row->content)
                || memcmp(source_bytes(found->content), source_bytes(row->content), source_length(row->content)))) { return false; }
    }
    *path = NULL; return true;
}
