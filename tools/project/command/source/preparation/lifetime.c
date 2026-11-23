/** @file
 * @brief Own command preparation children in dependency order and retire all failed prefixes.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03)
 * keeps borrowed snapshots alive until every plan/client is retired.
 */
#include "internal.h"
#include <string.h>
void project_source_preparation_init(ProjectSourcePreparation *owner, SourceAllocator allocator) {
    memset(owner, 0, sizeof(*owner)); source_arena_init(&owner->arena, allocator);
    source_root_capture_init(&owner->configured, allocator);
    source_tree_selection_init(&owner->source, allocator);
    source_tree_placement_init(&owner->initial_placement, allocator);
    source_tree_placement_init(&owner->placement, allocator);
    project_source_root_settings_init(&owner->root_settings, allocator);
    project_source_augmentation_init(&owner->augmentation, allocator);
    source_workspace_init(&owner->workspace, allocator);
    source_analysis_init(&owner->original, allocator);
    source_removal_selection_init(&owner->removal, allocator);
    source_clone_map_init(&owner->mains, allocator, &owner->configured);
    source_file_map_init(&owner->files, allocator);
    source_observed_image_init(&owner->observed, allocator);
    source_operation_complete_init(&owner->complete, allocator);
    source_compiler_client_init(&owner->final, allocator);
}
/** The complete certificate borrows the final client despite being constructed
 * earlier. Retire that borrower first, then the client, and finally captured
 * inputs under [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03).
 */
int project_source_preparation_destroy(ProjectSourcePreparation *owner) {
    source_operation_complete_destroy(&owner->complete);
    source_compiler_client_destroy(&owner->final);
    source_observed_image_destroy(&owner->observed);
    source_file_map_destroy(&owner->files); source_clone_map_destroy(&owner->mains);
    source_removal_selection_destroy(&owner->removal);
    source_analysis_destroy(&owner->original);
    int error = source_workspace_destroy(&owner->workspace);
    source_tree_placement_destroy(&owner->placement);
    project_source_augmentation_destroy(&owner->augmentation);
    project_source_root_settings_destroy(&owner->root_settings);
    source_tree_placement_destroy(&owner->initial_placement);
    source_tree_selection_destroy(&owner->source);
    source_root_capture_destroy(&owner->configured); source_arena_destroy(&owner->arena);
    memset(owner, 0, sizeof(*owner)); return error;
}
