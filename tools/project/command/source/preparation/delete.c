/** @file
 * @brief Derive deletion from actual original material and explicit root settings through the shared owner.
 * [TOOL-03](../../../../../spec/implementation/tools-and-conformance.md#tool-03)
 * checks the original cut; [TOOL-01](../../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * removes settings only for roots actually removed with the selected tree.
 */
#include "internal.h"
#include <string.h>
/** Exact physical membership, not a logical prefix or supplier-selection guess. */
static bool removed(const SourceRemovalSelection *removal, const char *path) {
    for (size_t i = 0; i < removal->path_count; i++) { if (!strcmp(removal->paths[i], path)) { return true; } }
    return false;
}
/** Retain every surviving setting; observe stable physical parents separately so
 * deleting a semantic root never invents a parent namespace or missing metadata. */
static ProjectSourcePreparationStatus settings(ProjectSourcePreparation *owner, SourceImageBounds *bounds) {
    size_t count = owner->configured.root_count;
    if (count > SIZE_MAX / sizeof(ModuleRoot) || count > SIZE_MAX / sizeof(SourceOperationRootDisposition)
        || count > SIZE_MAX / sizeof(const char *)) { return PROJECT_SOURCE_PREPARATION_MEMORY; }
    owner->final_roots = count ? source_arena_allocate(&owner->arena, count * sizeof(*owner->final_roots)) : NULL;
    owner->root_originals = count ? source_arena_allocate(&owner->arena, count * sizeof(*owner->root_originals)) : NULL;
    const char **parents = count ? source_arena_allocate(&owner->arena, count * sizeof(*parents)) : NULL;
    if (count && (!owner->final_roots || !owner->root_originals || !parents)) { return PROJECT_SOURCE_PREPARATION_MEMORY; }
    for (size_t i = 0; i < count; i++) {
        const ModuleRoot *root = &owner->configured.roots[i];
        const char *slash = strrchr(root->path, '/');
        size_t length = slash == root->path ? 1 : (size_t)(slash - root->path);
        char *parent = source_arena_allocate(&owner->arena, length + 1);
        if (!parent) { return PROJECT_SOURCE_PREPARATION_MEMORY; }
        memcpy(parent, root->path, length); parent[length] = 0; parents[i] = parent;
        if (removed(&owner->removal, root->path)) { owner->root_originals[i] = (SourceOperationRootDisposition){SOURCE_OPERATION_ROOT_DROP, SIZE_MAX}; }
        else {
            owner->root_originals[i] = (SourceOperationRootDisposition){SOURCE_OPERATION_ROOT_KEEP, owner->final_root_count};
            owner->final_roots[owner->final_root_count++] = *root;
        }
    }
    *bounds = (SourceImageBounds){parents, count}; return PROJECT_SOURCE_PREPARATION_OK;
}
ProjectSourcePreparationStatus project_source_preparation_delete(ProjectSourcePreparation *owner) {
    if (source_removal_selection_build(&owner->removal, &owner->source, &owner->original,
        owner->inputs.declared) != SOURCE_REMOVAL_SELECTION_OK) {
        owner->failed_path = owner->removal.failed_path; return PROJECT_SOURCE_PREPARATION_REMOVAL;
    }
    SourceImageBounds bounds;
    ProjectSourcePreparationStatus status = settings(owner, &bounds);
    if (status != PROJECT_SOURCE_PREPARATION_OK) { return status; }
    SourceRemovalMaterial material;
    if (source_removal_material_prepare(&owner->arena, &owner->removal, &material) != SOURCE_REMOVAL_MATERIAL_OK) { return PROJECT_SOURCE_PREPARATION_MEMORY; }
    if (source_observed_image_build(&owner->observed, &owner->configured, bounds, material.requests, material.count) != SOURCE_OBSERVATION_OK) {
        owner->failed_path = owner->observed.failed_path; return PROJECT_SOURCE_PREPARATION_OBSERVATION;
    }
    if (source_clone_map_seal(&owner->mains) != SOURCE_CLONE_OK || source_file_map_seal(&owner->files) != SOURCE_FILE_MAP_OK) { return PROJECT_SOURCE_PREPARATION_MAPPING; }
    owner->intent = (SourceOperationIntent){.kind = SOURCE_OPERATION_DELETE, .source = &owner->source, .source_removal = &owner->removal};
    owner->writes = (SourceOperationWritesInputs){.observed = &owner->observed, .mains = &owner->mains, .files = &owner->files, .removal = &owner->removal};
    SourceOperationCompleteInputs inputs = {.writes = &owner->writes, .intent = &owner->intent,
        .original = &owner->original, .configured = &owner->configured,
        .final_roots = owner->final_roots, .final_root_count = owner->final_root_count,
        .inputs = owner->inputs.inputs, .clients = owner->inputs.clients,
        .root_originals = owner->root_originals, .root_original_count = owner->configured.root_count};
    return source_operation_complete_prepare(&owner->complete, &inputs) == SOURCE_OPERATION_COMPLETE_OK
        ? PROJECT_SOURCE_PREPARATION_OK : PROJECT_SOURCE_PREPARATION_COMPLETE;
}
