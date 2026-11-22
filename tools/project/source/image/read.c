/** @file
 * @brief Borrow exact captured image bytes without reading or manufacturing file contents.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * distinguishes unavailable bytes from an observed empty file; the common catalog
 * deep-copies successful borrowed text under the requested final physical key.
 */
#include "internal.h"
#include <string.h>
SourceStatus source_image_read(void *context, const char *path, const SourceText **output) {
    const SourceCatalogImage *image = context;
    if (!image || !image->valid || !path || !output) { return SOURCE_INVALID_ARGUMENT; }
    const SourceCatalogImageEntry *entry = source_image_find(image, path, strlen(path));
    if (!entry || !entry->metadata_captured || !S_ISREG(entry->metadata.st_mode) || !entry->content) {
        return SOURCE_IO_ERROR;
    }
    *output = entry->content; return SOURCE_OK;
}
