/** @file
 * @brief Assign distinct symbolic entry identities in a finite immutable image.
 * [SRC-02](../../../../../spec/language/source-and-names.md#src-02)
 * distinguishes physical identity aliases; [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * plans new entries without predicting actual host inodes.
 */
#include "internal.h"
#include <stdint.h>
#include <string.h>
const SourceCatalogImageEntry *source_copy_image_original(const SourceCopyImageBuilder *builder, const char *path) {
    for (size_t i = 0; i < builder->original->count; i++) {
        if (!strcmp(builder->original->entries[i].path, path)) { return &builder->original->entries[i]; }
    }
    return NULL;
}
SourceImageChange *source_copy_image_change(const SourceCopyImageBuilder *builder, const char *path) {
    for (size_t i = 0; i < builder->count; i++) { if (!strcmp(builder->changes[i].entry.path, path)) { return &builder->changes[i]; } }
    return NULL;
}
SourceCopyImageStatus source_copy_image_identity(const SourceCopyImageBuilder *builder, struct stat *metadata) {
    for (uintmax_t label = 1;; label++) {
        if ((uintmax_t)(ino_t)label != label) { return SOURCE_COPY_IMAGE_LIMIT; }
        bool used = false;
        for (size_t i = 0; i < builder->original->count; i++) {
            const SourceCatalogImageEntry *entry = &builder->original->entries[i];
            if (entry->metadata_captured && entry->metadata.st_dev == 0 && (uintmax_t)entry->metadata.st_ino == label) { used = true; }
        }
        for (size_t i = 0; i < builder->count; i++) {
            const SourceImageChange *change = &builder->changes[i];
            if (change->kind != SOURCE_IMAGE_REMOVE && change->entry.metadata_captured
                && change->entry.metadata.st_dev == 0 && (uintmax_t)change->entry.metadata.st_ino == label) { used = true; }
        }
        if (!used) { metadata->st_dev = 0; metadata->st_ino = (ino_t)label; return SOURCE_COPY_IMAGE_OK; }
        if (label == UINTMAX_MAX) { return SOURCE_COPY_IMAGE_LIMIT; }
    }
}
