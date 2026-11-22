/** @file
 * @brief Demand and compare exact captured removal material, including opaque bytes.
 * [TOOL-03](../../../../../spec/implementation/tools-and-conformance.md#tool-03)
 * protects outside material; [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * refuses changed captured inputs rather than replacing the original baseline.
 */
#include "internal.h"
#include <stdint.h>
#include <string.h>
SourceRemovalImageStatus source_removal_image_requests(SourceRemovalImage *image, const SourceObservationRequest **output) {
    size_t count = 0;
    for (const SourceRemovalSelectedFile *file = image->selection->selected; file; file = file->next) {
        if (count == SIZE_MAX) { return SOURCE_REMOVAL_IMAGE_MEMORY; } count++;
    }
    if (count != image->selection->path_count) { return SOURCE_REMOVAL_IMAGE_INVALID; }
    if (count > SIZE_MAX / sizeof(SourceObservationRequest)) { return SOURCE_REMOVAL_IMAGE_MEMORY; }
    SourceObservationRequest *requests = count ? source_arena_allocate(&image->arena, count * sizeof(*requests)) : NULL;
    if (count && !requests) { return SOURCE_REMOVAL_IMAGE_MEMORY; }
    size_t index = 0;
    for (const SourceRemovalSelectedFile *file = image->selection->selected; file; file = file->next) {
        requests[index++] = (SourceObservationRequest){file->file->file.path,
            file->file->directory ? SOURCE_OBSERVE_CHILDREN : SOURCE_OBSERVE_CONTENT, false};
    }
    *output = requests; return SOURCE_REMOVAL_IMAGE_OK;
}
/** Find the exact demanded observation, without suffix/basename correspondence. */
static const SourceCatalogImageEntry *observed(const SourceRemovalImage *image, const char *path) {
    for (size_t i = 0; i < image->observed.image.count; i++) {
        if (!strcmp(image->observed.image.entries[i].path, path)) { return &image->observed.image.entries[i]; }
    }
    return NULL;
}
SourceRemovalImageStatus source_removal_image_changes(SourceRemovalImage *image, const SourceImageChange **output) {
    size_t count = image->selection->path_count;
    if (count > SIZE_MAX / sizeof(SourceImageChange)) { return SOURCE_REMOVAL_IMAGE_MEMORY; }
    SourceImageChange *changes = count ? source_arena_allocate(&image->arena, count * sizeof(*changes)) : NULL;
    if (count && !changes) { return SOURCE_REMOVAL_IMAGE_MEMORY; }
    size_t index = 0;
    for (const SourceRemovalSelectedFile *file = image->selection->selected; file; file = file->next) {
        const AcquisitionUnitEntry *old = file->file;
        const SourceCatalogImageEntry *now = observed(image, old->file.path);
        image->failed_path = old->file.path;
        if (!now || !now->metadata_captured || old->directory != (S_ISDIR(now->metadata.st_mode) != 0) ||
            (!old->directory && (!now->content || source_length(now->content) != source_length(old->file.content) ||
                memcmp(source_bytes(now->content), source_bytes(old->file.content), source_length(now->content))))) {
            return SOURCE_REMOVAL_IMAGE_CHANGED;
        }
        changes[index++] = (SourceImageChange){SOURCE_IMAGE_REMOVE, {.path = old->file.path}};
    }
    image->failed_path = NULL; *output = changes; return SOURCE_REMOVAL_IMAGE_OK;
}
