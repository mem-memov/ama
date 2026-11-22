/** @file
 * @brief Retain exact named observations and reject contradictory repeated metadata.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * keeps absence/unavailable data distinct; [SRC-02](../../../../spec/language/source-and-names.md#src-02)
 * gives device/inode identity and file/directory type explicit significance.
 */
#include "internal.h"
#include <stdint.h>
#include <string.h>
SourceObservedEntry *source_observation_find(SourceObservedImage *observed, const char *path) {
    for (SourceObservedEntry *entry = observed->first; entry; entry = entry->next) {
        if (!strcmp(entry->value.path, path)) { return entry; }
    }
    return NULL;
}
ModuleStatus source_observation_fail(SourceObservedImage *observed, ModuleCatalog *catalog,
    SourceObservationStatus status, ModuleStatus result, const char *path) {
    if (!observed->sealed && observed->status == SOURCE_OBSERVATION_OK) { observed->status = status; observed->failed_path = path; }
    catalog->issue = (ModuleIssue){.status = result, .path = path}; return result;
}
SourceObservedEntry *source_observation_entry(SourceObservedImage *observed, const char *path) {
    SourceObservedEntry *entry = source_observation_find(observed, path);
    if (entry) { return entry; }
    if (observed->count == SIZE_MAX) { observed->status = SOURCE_OBSERVATION_MEMORY; observed->failed_path = path; return NULL; }
    entry = source_arena_allocate(&observed->arena, sizeof(*entry));
    const char *key = entry ? source_arena_copy(&observed->arena, path) : NULL;
    if (!key) { observed->status = SOURCE_OBSERVATION_MEMORY; observed->failed_path = path; return NULL; }
    *entry = (SourceObservedEntry){.value = {.path = key}};
    if (observed->last) { observed->last->next = entry; } else { observed->first = entry; }
    observed->last = entry; observed->count++; return entry;
}
ModuleStatus source_observation_metadata(SourceObservedImage *observed, ModuleCatalog *catalog,
    const char *path, const struct stat *metadata) {
    SourceObservedEntry *entry = source_observation_entry(observed, path);
    if (!entry) { return source_observation_fail(observed, catalog, SOURCE_OBSERVATION_MEMORY, MODULE_OUT_OF_MEMORY, path); }
    if (entry->value.metadata_captured && (entry->value.metadata.st_dev != metadata->st_dev ||
        entry->value.metadata.st_ino != metadata->st_ino || entry->value.metadata.st_mode != metadata->st_mode)) {
        return source_observation_fail(observed, catalog, SOURCE_OBSERVATION_CHANGED, MODULE_IO_ERROR, entry->value.path);
    }
    entry->value.metadata = *metadata; entry->value.metadata_captured = true; return MODULE_OK;
}
