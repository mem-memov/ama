/** @file
 * @brief Carry only genuine mapped raw directory roots, preserving outside settings and namespace family.
 * [TOOL-01](../../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * makes COPY retain originals and MOVE relocate their actual selected settings.
 */
#include "internal.h"
#include <string.h>
/** Exact directory membership supplies provenance, never a lexical prefix guess. */
static const char *carried(const ProjectSourceRootSettings *owner, const char *path) {
    for (const SourceTreePlacementEntry *row = owner->placement->entries; row; row = row->next) {
        if (row->original->directory && !strcmp(row->original->file.path, path)) { return row->path; }
    }
    return NULL;
}
ProjectSourceRootSettingsStatus project_source_root_settings_originals(ProjectSourceRootSettings *owner, SourceOperationKind kind) {
    for (size_t i = 0; i < owner->configured->root_count; i++) {
        ModuleRoot root = owner->configured->roots[i]; const char *path = carried(owner, root.path);
        if (path && kind == SOURCE_OPERATION_MOVE) { root.path = path; }
        size_t final;
        ProjectSourceRootSettingsStatus status = project_source_root_settings_add(owner, root, false, &final);
        if (status != PROJECT_SOURCE_ROOTS_OK) { return status; }
        owner->originals[i] = (SourceOperationRootDisposition){path && kind == SOURCE_OPERATION_MOVE ? SOURCE_OPERATION_ROOT_CARRY : SOURCE_OPERATION_ROOT_KEEP, final};
    }
    if (kind == SOURCE_OPERATION_MOVE) { return PROJECT_SOURCE_ROOTS_OK; }
    for (size_t i = 0; i < owner->configured->root_count; i++) {
        ModuleRoot root = owner->configured->roots[i]; const char *path = carried(owner, root.path);
        if (!path) { continue; }
        root.path = path; size_t final, before = owner->final_count;
        ProjectSourceRootSettingsStatus status = project_source_root_settings_add(owner, root, false, &final);
        if (status != PROJECT_SOURCE_ROOTS_OK) { return status; }
        if (owner->final_count != before) { owner->additions[owner->addition_count++] = (SourceOperationRootAddition){final, i}; }
    }
    return PROJECT_SOURCE_ROOTS_OK;
}
