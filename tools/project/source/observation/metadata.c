/** @file
 * @brief Capture exact local root/child metadata inside the already authorized original domain.
 * [SRC-02](../../../../spec/language/source-and-names.md#src-02) rejects symlink
 * and spelling aliases; [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * refuses disagreements with an already observed complete directory listing.
 */
#include "internal.h"
#include <string.h>
ModuleStatus source_observation_root(void *context, ModuleCatalog *catalog, const char *path, struct stat *output) {
    SourceObservedImage *observed = context;
    if (observed->sealed || observed->status != SOURCE_OBSERVATION_OK) {
        return source_observation_fail(observed, catalog, SOURCE_OBSERVATION_INVALID, MODULE_INVALID_PATH, path);
    }
    bool covered = false;
    for (size_t i = 0; i < observed->original->root_count; i++) { covered |= source_image_below(path, observed->original->roots[i].path); }
    if (!covered) { return source_observation_fail(observed, catalog, SOURCE_OBSERVATION_INVALID, MODULE_INVALID_PATH, path); }
    SourceObservedEntry *entry = source_observation_find(observed, path);
    if (entry && entry->value.metadata_captured) {
        if (!S_ISDIR(entry->value.metadata.st_mode)) {
            return source_observation_fail(observed, catalog, SOURCE_OBSERVATION_PHYSICAL, MODULE_UNSUPPORTED_PATH, path);
        }
        *output = entry->value.metadata; return MODULE_OK;
    }
    SourcePhysicalPath physical = {0};
    SourcePhysicalStatus status = source_physical_resolve(&observed->arena, path, SOURCE_PHYSICAL_DIRECTORY,
        false, &physical, &observed->physical_issue);
    if (status != SOURCE_PHYSICAL_OK) {
        return source_observation_fail(observed, catalog,
            status == SOURCE_PHYSICAL_STORAGE ? SOURCE_OBSERVATION_MEMORY : SOURCE_OBSERVATION_PHYSICAL,
            status == SOURCE_PHYSICAL_STORAGE ? MODULE_OUT_OF_MEMORY : status == SOURCE_PHYSICAL_MISSING ? MODULE_MISSING :
            status == SOURCE_PHYSICAL_UNSUPPORTED ? MODULE_UNSUPPORTED_PATH : MODULE_IO_ERROR,
            observed->physical_issue.path ? observed->physical_issue.path : path);
    }
    ModuleStatus recorded = source_observation_metadata(observed, catalog, path, &physical.metadata);
    if (recorded == MODULE_OK) { *output = physical.metadata; }
    return recorded;
}
ModuleStatus source_observation_child(void *context, ModuleCatalog *catalog, const char *parent,
    const char *name, const char *full, struct stat *output) {
    SourceObservedImage *observed = context; const char **names = NULL; size_t count = 0;
    ModuleStatus status = source_observation_entries(context, catalog, parent, &names, &count);
    if (status != MODULE_OK) { return status; }
    (void)names; (void)count;
    struct stat metadata; PathProbe probe = path_exact_child(parent, name, full, &metadata);
    SourceObservedEntry *entry = source_observation_find(observed, full);
    if ((probe == PATH_FOUND && !entry) || (probe == PATH_MISSING && entry)) {
        return source_observation_fail(observed, catalog, SOURCE_OBSERVATION_CHANGED, MODULE_IO_ERROR, full);
    }
    if (probe == PATH_MISSING) { catalog->issue = (ModuleIssue){.status = MODULE_MISSING, .path = full}; return MODULE_MISSING; }
    if (probe != PATH_FOUND) {
        return source_observation_fail(observed, catalog, SOURCE_OBSERVATION_PHYSICAL,
            probe == PATH_UNSUPPORTED ? MODULE_UNSUPPORTED_PATH : MODULE_IO_ERROR, full);
    }
    status = source_observation_metadata(observed, catalog, full, &metadata);
    if (status == MODULE_OK) { *output = metadata; }
    return status;
}
