/** @file
 * @brief Match exact prepared physical keys and configured root boundaries without normalization.
 * [SRC-02](../../../../spec/language/source-and-names.md#src-02) preserves physical
 * alias provenance; [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * forbids guessed prefix rewrites or hidden physical fallback.
 */
#include "internal.h"
#include <string.h>
const SourceCatalogImageEntry *source_image_find(const SourceCatalogImage *image,
    const char *path, size_t length) {
    for (size_t i = 0; i < image->count; i++) {
        if (strlen(image->entries[i].path) == length && !memcmp(image->entries[i].path, path, length)) {
            return &image->entries[i];
        }
    }
    return NULL;
}
bool source_image_below(const char *path, const char *root) {
    if (!path || !root || path[0] != '/' || root[0] != '/') { return false; }
    size_t length = strlen(root);
    return !strncmp(path, root, length) && (!path[length] || root[length - 1] == '/' || path[length] == '/');
}
