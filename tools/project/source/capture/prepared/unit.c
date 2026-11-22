/** @file
 * @brief Own complete selected image units under final physical source/companion keys.
 * [PKG-07](../../../../../spec/project/packages.md#pkg-07) fixes exact main/dot
 * membership; [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * forbids treating unread native/unknown companions as absent or using host files.
 */
#include "internal.h"
#include <string.h>
/** The complete unit contains one exact main and basename-dot-nonempty files. */
static bool member(const char *path, const char *main) {
    size_t length = strlen(main);
    return !strcmp(path, main) || (!strncmp(path, main, length) && path[length] == '.' && path[length + 1]);
}
/** Copy exact companion bytes under their final key; the catalog already owns main text. */
static SourceImageCaptureStatus append(SourceImageCapture *capture, const ModuleSource *source,
    const SourceCatalogImageEntry *entry, AcquisitionUnitEntry **files) {
    capture->failed_path = entry->path;
    if (!entry->metadata_captured) { return SOURCE_IMAGE_CAPTURE_UNAVAILABLE; }
    if (!S_ISREG(entry->metadata.st_mode)) { return SOURCE_IMAGE_CAPTURE_KIND; }
    if (!entry->content) { return SOURCE_IMAGE_CAPTURE_UNAVAILABLE; }
    const SourceText *content = source->text;
    if (strcmp(entry->path, source_path(source->text))) {
        capture->file_status = source_manager_add(&capture->files, entry->path,
            source_bytes(entry->content), source_length(entry->content), &content);
        if (capture->file_status != SOURCE_OK) { return SOURCE_IMAGE_CAPTURE_MEMORY; }
    }
    AcquisitionUnitEntry *file = source_arena_allocate(&capture->arena, sizeof(*file));
    if (!file) { return SOURCE_IMAGE_CAPTURE_MEMORY; }
    *file = (AcquisitionUnitEntry){.file = {source_path(content), content, DELIVERY_FILE_PRESENT, PATH_FOUND}};
    AcquisitionUnitEntry **position = files;
    while (*position && strcmp((*position)->file.path, file->file.path) < 0) { position = &(*position)->next; }
    file->next = *position; *position = file;
    return SOURCE_IMAGE_CAPTURE_OK;
}
SourceImageCaptureStatus source_image_capture_unit(SourceImageCapture *capture,
    const ModuleSource *source, SourceCapturedUnit **output) {
    AcquisitionUnitEntry *files = NULL;
    for (size_t i = 0; i < capture->image->count; i++) {
        const SourceCatalogImageEntry *entry = &capture->image->entries[i];
        if (!member(entry->path, source_path(source->text))) { continue; }
        SourceImageCaptureStatus status = append(capture, source, entry, &files);
        if (status != SOURCE_IMAGE_CAPTURE_OK) { return status; }
    }
    SourceCapturedUnit *unit = source_arena_allocate(&capture->arena, sizeof(*unit));
    if (!unit) { return SOURCE_IMAGE_CAPTURE_MEMORY; }
    *unit = (SourceCapturedUnit){.source = source, .files = files}; *output = unit;
    return SOURCE_IMAGE_CAPTURE_OK;
}
