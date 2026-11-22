/** @file
 * @brief Capture a required complete associated test directory through the structural walker.
 * [PKG-08](../../../../../spec/project/packages.md#pkg-08) retains whole mirrored
 * test directories; [PKG-07](../../../../../spec/project/packages.md#pkg-07)
 * requires exact bytes and refuses missing required inputs before publication.
 */
#include "internal.h"
#include <string.h>
SourceStatus acquisition_directory_capture(SourceArena *arena, SourceManager *texts,
    const char *parent, const char *name, const AcquisitionUnitEntry **output, const char **failed_path) {
    if (!arena || !texts || !parent || !*parent || !name || !*name || strchr(name, '/') ||
        !strcmp(name, ".") || !strcmp(name, "..") || !output || !failed_path) {
        return SOURCE_INVALID_ARGUMENT;
    }
    AcquisitionUnitCapture capture = {.arena = arena, .texts = texts};
    module_catalog_init(&capture.entries, arena->allocator);
    SourceStatus status = acquisition_unit_tree(&capture, parent, name);
    if (status == SOURCE_OK && !capture.first) { status = SOURCE_IO_ERROR; }
    module_catalog_destroy(&capture.entries);
    if (status == SOURCE_OK) { *output = capture.first; *failed_path = NULL; }
    else { *failed_path = capture.failed; }
    return status;
}
