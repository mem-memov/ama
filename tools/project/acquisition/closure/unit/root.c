/** @file
 * @brief Retain independently resolved directory operands without a synthetic parent/basename.
 * [TOOL-01](../../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * includes explicit directory trees; [SRC-02](../../../../../spec/language/source-and-names.md#src-02)
 * rejects changed root identity and unsupported links; [PKG-07](../../../../../spec/project/packages.md#pkg-07)
 * preserves empty directories and complete raw contents.
 */
#include <string.h>
#include "internal.h"
SourceStatus acquisition_directory_capture_root(SourceArena *arena, SourceManager *texts,
    const SourcePhysicalPath *root, const AcquisitionUnitEntry **output, const char **failed_path) {
    if (!arena || !texts || !root || !root->path || !*root->path || !root->present
        || !S_ISDIR(root->metadata.st_mode) || !output || !failed_path) { return SOURCE_INVALID_ARGUMENT; }
    const char *path = source_arena_copy(arena, root->path);
    if (!path) { return SOURCE_OUT_OF_MEMORY; }
    *failed_path = path;
    struct stat metadata;
    if (lstat(path, &metadata) != 0 || !S_ISDIR(metadata.st_mode)
        || metadata.st_dev != root->metadata.st_dev || metadata.st_ino != root->metadata.st_ino) { return SOURCE_IO_ERROR; }
    AcquisitionUnitCapture capture = {.arena = arena, .texts = texts, .failed = path};
    module_catalog_init(&capture.entries, arena->allocator);
    ModuleStatus remembered = module_remember_directory_metadata(&capture.entries, path, &metadata);
    SourceStatus status = remembered == MODULE_OK ? SOURCE_OK
        : remembered == MODULE_OUT_OF_MEMORY ? SOURCE_OUT_OF_MEMORY : SOURCE_IO_ERROR;
    AcquisitionUnitEntry *entry = status == SOURCE_OK ? source_arena_allocate(arena, sizeof(*entry)) : NULL;
    if (status == SOURCE_OK && !entry) { status = SOURCE_OUT_OF_MEMORY; }
    if (status == SOURCE_OK) {
        *entry = (AcquisitionUnitEntry){.directory = true,
            .file = {.path = path, .state = DELIVERY_FILE_PRESENT, .issue = PATH_FOUND}};
        capture.first = entry;
        status = acquisition_unit_walk(&capture, path);
    }
    module_catalog_destroy(&capture.entries);
    if (status == SOURCE_OK) { *output = capture.first; *failed_path = NULL; }
    else { *failed_path = capture.failed; }
    return status;
}
