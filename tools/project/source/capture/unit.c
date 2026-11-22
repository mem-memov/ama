/** @file
 * @brief Capture raw unit bytes and refuse contradictory observations of one main or INI.
 * [PKG-07](../../../../spec/project/packages.md#pkg-07) retains all same-name companions;
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * forbids convenient recapture after a detected change.
 */
#include <string.h>
#include "internal.h"
/** Compare separately owned exact source observations, including presence under
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04).
 */
static bool same(const SourceText *left, const SourceText *right) {
    if (!left || !right) { return left == right; }
    return !strcmp(source_path(left), source_path(right)) && source_length(left) == source_length(right)
        && !memcmp(source_bytes(left), source_bytes(right), source_length(left));
}
/** Require the originally indexed main rather than a replacement inode under
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04).
 */
static bool identity(const ModuleSource *source, const char *parent, const char *basename) {
    struct stat metadata;
    return path_exact_child(parent, basename, source_path(source->text), &metadata) == PATH_FOUND
        && S_ISREG(metadata.st_mode) && metadata.st_dev == source->device && metadata.st_ino == source->inode;
}
SourceCaptureStatus source_capture_unit(SourceRootCapture *capture, const ModuleSource *source, SourceCapturedUnit **output) {
    const char *path = source_path(source->text), *slash = strrchr(path, '/');
    const char *basename = slash ? slash + 1 : path;
    capture->failed_path = path;
    char *parent = source_arena_copy(&capture->arena, path);
    if (!parent) { return SOURCE_CAPTURE_MEMORY; }
    if (slash) { parent[slash - path] = 0; if (!*parent) { parent = "/"; } }
    else { parent = "."; }
    capture->failed_path = path;
    if (!identity(source, parent, basename)) { return SOURCE_CAPTURE_CHANGED; }
    const AcquisitionUnitEntry *files = NULL;
    capture->file_status = acquisition_unit_capture(&capture->arena, &capture->files, parent, basename,
        ACQUISITION_CONTEXT, &files, &capture->failed_path);
    if (capture->file_status != SOURCE_OK) { return SOURCE_CAPTURE_FILES; }
    const SourceText *main = NULL, *configuration = NULL;
    for (const AcquisitionUnitEntry *file = files; file; file = file->next) {
        if (!strcmp(file->file.path, path)) { main = file->file.content; }
        if (!strcmp(file->file.path, source->configuration_path)) { configuration = file->file.content; }
    }
    capture->failed_path = path;
    if (!identity(source, parent, basename) || !same(main, source->text)) { return SOURCE_CAPTURE_CHANGED; }
    capture->failed_path = source->configuration_path;
    if (!same(configuration, source->configuration)) { return SOURCE_CAPTURE_CHANGED; }
    SourceCapturedUnit *unit = source_arena_allocate(&capture->arena, sizeof(*unit));
    if (!unit) { return SOURCE_CAPTURE_MEMORY; }
    *unit = (SourceCapturedUnit){.source = source, .files = files}; *output = unit;
    return SOURCE_CAPTURE_OK;
}
