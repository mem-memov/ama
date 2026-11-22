/** @file
 * @brief Capture complete original names once, keeping unread child metadata explicitly unavailable.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires complete membership observations; [PKG-02](../../../../spec/project/packages.md#pkg-02)
 * must skip ignored versions without forcing reads of their bodies or metadata.
 */
#include "internal.h"
ModuleStatus source_observation_entries(void *context, ModuleCatalog *catalog, const char *path,
    const char ***output, size_t *count) {
    SourceObservedImage *observed = context; struct stat metadata;
    ModuleStatus status = source_observation_root(context, catalog, path, &metadata);
    if (status != MODULE_OK) { return status; }
    SourceObservedEntry *directory = source_observation_find(observed, path);
    if (!directory->value.children_complete) {
        const char **names = NULL; size_t length = 0;
        status = module_catalog_entries(&observed->physical, path, &names, &length);
        if (status != MODULE_OK) {
            return source_observation_fail(observed, catalog,
                status == MODULE_OUT_OF_MEMORY ? SOURCE_OBSERVATION_MEMORY : SOURCE_OBSERVATION_PHYSICAL,
                status, observed->physical.issue.path ? observed->physical.issue.path : path);
        }
        for (size_t i = 0; i < length; i++) {
            const char *key = source_arena_join(&observed->arena, path, "/", names[i]);
            if (!key || !source_observation_entry(observed, key)) {
                return source_observation_fail(observed, catalog, SOURCE_OBSERVATION_MEMORY, MODULE_OUT_OF_MEMORY, path);
            }
        }
        directory->names = names; directory->name_count = length; directory->value.children_complete = true;
    }
    *output = directory->names; *count = directory->name_count; return MODULE_OK;
}
