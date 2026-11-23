/** @file
 * @brief Prepare explicit final roots and common augmented-original placement before compiler analysis.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * requires newly covered existing inputs in the original domain before edits;
 * candidate settings never replace the shared complete final root/material proof.
 */
#include "internal.h"
ProjectSourcePreparationStatus project_source_preparation_destination(ProjectSourcePreparation *owner) {
    const ProjectSourceRequest *request = owner->inputs.request;
    if (source_physical_destination(&owner->arena, request->destination, SOURCE_PHYSICAL_ENDPOINT,
        &owner->destination, &owner->destination_issue) != SOURCE_PHYSICAL_OK) {
        owner->failed_path = owner->destination_issue.path; return PROJECT_SOURCE_PREPARATION_DESTINATION;
    }
    if (!S_ISDIR(owner->source.operand.metadata.st_mode) && owner->destination.directory_required) {
        owner->failed_path = owner->destination.path; return PROJECT_SOURCE_PREPARATION_DESTINATION;
    }
    if (source_tree_placement_build(&owner->initial_placement, &owner->source, owner->destination.path) != SOURCE_PLACEMENT_OK) {
        owner->failed_path = owner->initial_placement.failed_path; return PROJECT_SOURCE_PREPARATION_PLACEMENT;
    }
    if (project_source_root_settings_build(&owner->root_settings, &owner->configured, &owner->initial_placement,
        request->kind, request->destination_roots, request->destination_root_count) != PROJECT_SOURCE_ROOTS_OK) {
        owner->failed_path = owner->root_settings.failed_path; return PROJECT_SOURCE_PREPARATION_ROOTS;
    }
    if (project_source_augmentation_build(&owner->augmentation, &owner->root_settings) != PROJECT_SOURCE_AUGMENTATION_OK) {
        owner->failed_path = owner->augmentation.failed_path; return PROJECT_SOURCE_PREPARATION_AUGMENTATION;
    }
    owner->original_capture = owner->augmentation.original; owner->selected = owner->augmentation.selected;
    if (source_tree_placement_build(&owner->placement, owner->selected, owner->destination.path) != SOURCE_PLACEMENT_OK) {
        owner->failed_path = owner->placement.failed_path; return PROJECT_SOURCE_PREPARATION_PLACEMENT;
    }
    return PROJECT_SOURCE_PREPARATION_OK;
}
