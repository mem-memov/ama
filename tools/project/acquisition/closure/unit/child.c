/** @file
 * @brief Snapshot one exact retained structural member and reject unsupported storage.
 * [PKG-07](../../../../../spec/project/packages.md#pkg-07) requires lossless files
 * and empty directories; [SRC-02](../../../../../spec/language/source-and-names.md#src-02)
 * prevents alias-driven unbounded directory traversal.
 */
#include "internal.h"
#include <string.h>
SourceStatus acquisition_unit_child(AcquisitionUnitCapture *capture, const char *parent,
    const char *name, bool optional, bool directory, AcquisitionUnitEntry **output) {
    const char *path = source_arena_join(capture->arena, parent,
        parent[strlen(parent) - 1] == '/' ? "" : "/", name);
    if (!path) { return SOURCE_OUT_OF_MEMORY; }
    capture->failed = path;
    struct stat metadata;
    PathProbe probe = path_exact_child(parent, name, path, &metadata);
    if (probe == PATH_MISSING && optional) { *output = NULL; return SOURCE_OK; }
    if (probe != PATH_FOUND || (directory ? !S_ISDIR(metadata.st_mode) : !S_ISREG(metadata.st_mode))) {
        return SOURCE_IO_ERROR;
    }
    AcquisitionUnitEntry *entry = source_arena_allocate(capture->arena, sizeof(*entry));
    if (!entry) { return SOURCE_OUT_OF_MEMORY; }
    *entry = (AcquisitionUnitEntry){.directory = directory,
        .file = {.path = path, .state = DELIVERY_FILE_PRESENT, .issue = PATH_FOUND}};
    if (directory) {
        ModuleStatus status = module_remember_directory_metadata(&capture->entries, path, &metadata);
        if (status != MODULE_OK) { return status == MODULE_OUT_OF_MEMORY ? SOURCE_OUT_OF_MEMORY : SOURCE_IO_ERROR; }
    } else {
        SourceStatus status = delivery_file_capture(capture->arena, capture->texts, parent, name, &entry->file);
        if (status != SOURCE_OK) { return status; }
        if (entry->file.state != DELIVERY_FILE_PRESENT) { return SOURCE_IO_ERROR; }
    }
    AcquisitionUnitEntry **position = &capture->first;
    while (*position && strcmp((*position)->file.path, path) < 0) { position = &(*position)->next; }
    entry->next = *position; *position = entry; *output = entry;
    return SOURCE_OK;
}
