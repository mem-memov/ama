/** @file
 * @brief Prove new-version absence from exact original directory observations.
 * [TOOL-01](../../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * permits initialization only for a wholly absent version, never repair of an
 * existing directory or inference through an unread/regular ancestor.
 */
#include "internal.h"
#include <string.h>
/** An exact missing key is proven only by the closest observed complete
 * directory ancestor. Existing, unknown or regular ancestors grant no repair. */
bool source_configuration_version_absent(const SourceCatalogImage *image, const char *path) {
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
