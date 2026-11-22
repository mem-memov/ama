/** @file
 * @brief Capture every original unit companion and reject added/removed same-module members.
 * [PKG-07](../../../../spec/project/packages.md#pkg-07) includes all dot companions;
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * refuses changed bytes/membership rather than adopting a newer original snapshot.
 */
#include "internal.h"
#include <string.h>
/** Retain an exact changed original key without refreshing its accepted capture. */
static SourceObservationStatus changed(SourceObservedImage *observed, const char *path) {
    observed->failed_path = path; return observed->status = SOURCE_OBSERVATION_CHANGED;
}
/** A present recorded same-module name must already belong to the original unit. */
static bool original_member(const SourceCapturedUnit *unit, const char *path) {
    for (const AcquisitionUnitEntry *file = unit->files; file; file = file->next) {
        if (!strcmp(file->file.path, path)) { return true; }
    }
    return false;
}
/** Fetch an original regular member through its complete parent observation. */
static SourceObservationStatus file(SourceObservedImage *observed, const AcquisitionUnitEntry *old) {
    const char *path = old->file.path;
    if (!source_observation_find(observed, path)) { return changed(observed, path); }
    char *parent = source_arena_copy(&observed->arena, path);
    if (!parent) { observed->failed_path = path; return observed->status = SOURCE_OBSERVATION_MEMORY; }
    char *slash = strrchr(parent, '/');
    const char *name = path + (slash - parent) + 1;
    if (slash == parent) { slash[1] = 0; } else { *slash = 0; }
    struct stat metadata;
    ModuleStatus probed = source_observation_child(observed, &observed->catalog, parent, name, path, &metadata);
    if (probed != MODULE_OK) { return observed->status == SOURCE_OBSERVATION_OK ? changed(observed, path) : observed->status; }
    if (!S_ISREG(metadata.st_mode)) { return changed(observed, path); }
    const SourceText *content = NULL;
    if (source_observation_read(observed, path, &content) != SOURCE_OK) { return observed->status; }
    if (source_length(content) != source_length(old->file.content) ||
        memcmp(source_bytes(content), source_bytes(old->file.content), source_length(content))) { return changed(observed, path); }
    return SOURCE_OBSERVATION_OK;
}
SourceObservationStatus source_observation_units(SourceObservedImage *observed) {
    for (const SourceCapturedUnit *unit = observed->original->units; unit; unit = unit->next) {
        const char *main = source_path(unit->source->text); size_t length = strlen(main);
        for (const SourceObservedEntry *entry = observed->first; entry; entry = entry->next) {
            const char *path = entry->value.path;
            if (!strncmp(path, main, length) && path[length] == '.' && path[length + 1] && !original_member(unit, path)) {
                return changed(observed, path);
            }
        }
        for (const AcquisitionUnitEntry *old = unit->files; old; old = old->next) {
            SourceObservationStatus status = file(observed, old);
            if (status != SOURCE_OBSERVATION_OK) { return status; }
        }
    }
    return SOURCE_OBSERVATION_OK;
}
