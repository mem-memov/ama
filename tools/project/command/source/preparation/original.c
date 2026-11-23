/** @file
 * @brief Capture explicit roots/source and independently analyze immutable original bytes.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * requires whole-domain original facts; [PROJECT-01](../../../../../spec/implementation/project-tools.md#project-01)
 * permits only analysis subprocesses, never executing the selected source module.
 */
#include "internal.h"
ProjectSourcePreparationStatus project_source_preparation_original(ProjectSourcePreparation *owner) {
    const ProjectSourceRequest *request = owner->inputs.request;
    size_t count = request->root_count;
    if (count > SIZE_MAX / sizeof(ModuleRoot) || count == SIZE_MAX) { return PROJECT_SOURCE_PREPARATION_MEMORY; }
    ModuleRoot *roots = count ? source_arena_allocate(&owner->arena, count * sizeof(*roots)) : NULL;
    if (count && !roots) { return PROJECT_SOURCE_PREPARATION_MEMORY; }
    for (size_t i = 0; i < count; i++) { roots[i] = request->roots[i].value; }
    if (source_root_capture_build(&owner->configured, roots, count) != SOURCE_CAPTURE_OK) {
        owner->failed_path = owner->configured.failed_path; return PROJECT_SOURCE_PREPARATION_CAPTURE;
    }
    if (source_tree_selection_read(&owner->source, &owner->configured, request->source) != SOURCE_TREE_OK) {
        owner->failed_path = owner->source.failed_path; return PROJECT_SOURCE_PREPARATION_SELECTION;
    }
    owner->original_capture = &owner->configured; owner->selected = &owner->source;
    if (request->kind != SOURCE_OPERATION_DELETE) {
        ProjectSourcePreparationStatus status = project_source_preparation_destination(owner);
        if (status != PROJECT_SOURCE_PREPARATION_OK) { return status; }
    }
    owner->mains.capture = owner->original_capture;
    size_t final = owner->root_settings.final_count;
    if (count > SIZE_MAX - 2 || final > SIZE_MAX - count - 2
        || count + final + 2 > SIZE_MAX / sizeof(const char *)) { return PROJECT_SOURCE_PREPARATION_MEMORY; }
    const char **protected = source_arena_allocate(&owner->arena, (count + final + 2) * sizeof(*protected));
    if (!protected) { return PROJECT_SOURCE_PREPARATION_MEMORY; }
    for (size_t i = 0; i < count; i++) { protected[i] = owner->configured.roots[i].path; }
    for (size_t i = 0; i < final; i++) { protected[count+i] = owner->root_settings.final[i].path; }
    protected[count+final] = owner->source.operand.path;
    size_t protected_count = count + final + 1;
    if (owner->destination.path) { protected[protected_count++] = owner->destination.path; }
    if (source_workspace_open(&owner->workspace, owner->inputs.temporary_parent, protected, protected_count, owner->inputs.control) != SOURCE_WORKSPACE_OK
        || source_workspace_snapshot(&owner->workspace, SOURCE_WORKSPACE_ORIGINAL, &owner->original_capture->view) != SOURCE_WORKSPACE_OK) {
        owner->failed_path = owner->workspace.issue.path; return PROJECT_SOURCE_PREPARATION_WORKSPACE;
    }
    if (source_analysis_check(&owner->original, &owner->original_capture->view, request->compiler,
        owner->workspace.files[SOURCE_WORKSPACE_ORIGINAL].entry.path) != SOURCE_ANALYSIS_OK) { return PROJECT_SOURCE_PREPARATION_ANALYSIS; }
    return PROJECT_SOURCE_PREPARATION_OK;
}
