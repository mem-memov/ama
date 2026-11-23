/** @file
 * @brief Prove exact path absence from complete immutable original directory observations.
 * [TOOL-01](../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * permits new-version initialization and needed context creation only where
 * complete observations prove absence, never through an unread/regular ancestor.
 */
#include "image.h"
#include <string.h>
/** An exact missing key is proven only by the closest observed complete
 * directory ancestor. Existing, unknown or regular ancestors grant no repair. */
bool source_image_path_absent(const SourceCatalogImage *image, const char *path) {
    if (!image || !image->valid || !path || path[0] != '/') { return false; }
    size_t length = strlen(path);
    for (;;) {
        for (size_t i = 0; i < image->count; i++) {
            const SourceCatalogImageEntry *entry = &image->entries[i];
            if (strlen(entry->path) == length && !memcmp(entry->path, path, length)) {
                return length != strlen(path) && entry->metadata_captured
                    && S_ISDIR(entry->metadata.st_mode) && entry->children_complete;
            }
        }
        if (length <= 1) { return false; }
        while (length && path[length - 1] != '/') { length--; }
        if (length > 1) { length--; }
    }
}
