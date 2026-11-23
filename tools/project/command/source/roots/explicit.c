/** @file
 * @brief Apply complete explicit destination-root scopes without inferring roots or merging exclusions.
 * [TOOL-01](../../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * requires an explicit unknown destination setting and preserved root family.
 */
#include "internal.h"
#include <string.h>
ProjectSourceRootSettingsStatus project_source_root_settings_explicit(ProjectSourceRootSettings *owner,
    const ProjectSourceRootRequest *requests, size_t count) {
    const char **keys = count ? source_arena_allocate(&owner->arena, count * sizeof(*keys)) : NULL;
    if (count && !keys) { return PROJECT_SOURCE_ROOTS_MEMORY; }
    for (size_t i = 0; i < count; i++) {
        ModuleRoot root = requests[i].value;
        if (!root.path || !*root.path || (root.excluded_count && !root.excluded)
            || (root.kind != MODULE_ROOT_PROJECT && root.kind != MODULE_ROOT_STANDARD)) { return PROJECT_SOURCE_ROOTS_INVALID; }
        SourcePhysicalPath path;
        if (source_physical_destination(&owner->arena, root.path, SOURCE_PHYSICAL_DIRECTORY, &path, &owner->physical_issue) != SOURCE_PHYSICAL_OK) {
            owner->failed_path = owner->physical_issue.path; return PROJECT_SOURCE_ROOTS_PHYSICAL;
        }
        /* Distinct raw arguments must not silently collapse after physical
         * normalization. The request parser already rejects equal raw keys. */
        for (size_t j = 0; j < i; j++) {
            if (!strcmp(keys[j], path.path)) { owner->failed_path = path.path; return PROJECT_SOURCE_ROOTS_CONFLICT; }
        }
        keys[i] = path.path; root.path = path.path; size_t final, before = owner->final_count;
        ProjectSourceRootSettingsStatus status = project_source_root_settings_add(owner, root, true, &final);
        if (status != PROJECT_SOURCE_ROOTS_OK) { return status; }
        if (before != owner->final_count) { owner->additions[owner->addition_count++] = (SourceOperationRootAddition){final, SIZE_MAX}; }
    }
    return PROJECT_SOURCE_ROOTS_OK;
}
