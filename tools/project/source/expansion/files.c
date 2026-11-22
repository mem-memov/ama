/** @file
 * @brief Retain complete structural bytes while rejecting changed original indexed units.
 * [PKG-07](../../../../spec/project/packages.md#pkg-07) includes opaque trees;
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * forbids silently substituting a changed original observation.
 */
#include <string.h>
#include "internal.h"
#include "../capture/match.h"
AcquisitionClosureStatus source_expansion_observed(SourceExpansion *expansion,
    const AcquisitionUnitEntry *files, const char *path, bool module) {
    size_t length = strlen(path);
    for (const SourceCapturedUnit *unit = expansion->capture->units; unit; unit = unit->next) {
        const char *candidate = source_path(unit->source->text);
        bool covered = module ? !strcmp(candidate, path) || (!strncmp(candidate, path, length)
            && candidate[length] == '+' && candidate[length + 1] == '/')
            : !strncmp(candidate, path, length) && candidate[length] == '/';
        if (covered && !source_captured_unit_matches(unit, files)) {
            expansion->failed_path = candidate;
            return ACQUISITION_CLOSURE_CONFLICT;
        }
    }
    return ACQUISITION_CLOSURE_OK;
}
AcquisitionClosureStatus source_expansion_files(SourceExpansion *expansion, const ModuleSource *source,
    const AcquisitionUnitEntry **output) {
    const char *path = source_path(source->text), *slash = strrchr(path, '/');
    if (!slash) { return ACQUISITION_CLOSURE_INVALID; }
    char *parent = source_arena_copy(&expansion->arena, path);
    if (!parent) { return ACQUISITION_CLOSURE_OUT_OF_MEMORY; }
    parent[slash - path] = 0;
    if (!*parent) { parent = "/"; }
    const AcquisitionUnitEntry *files = NULL;
    expansion->file_status = acquisition_unit_capture(&expansion->arena, &expansion->files,
        parent, slash + 1, ACQUISITION_ACTIVE, &files, &expansion->failed_path);
    if (expansion->file_status != SOURCE_OK) {
        return expansion->file_status == SOURCE_OUT_OF_MEMORY || expansion->file_status == SOURCE_LIMIT
            ? ACQUISITION_CLOSURE_OUT_OF_MEMORY : ACQUISITION_CLOSURE_PREPARATION_FAILED;
    }
    AcquisitionClosureStatus status = source_expansion_observed(expansion, files, path, true);
    if (status != ACQUISITION_CLOSURE_OK) { return status; }
    *output = files;
    return ACQUISITION_CLOSURE_OK;
}
