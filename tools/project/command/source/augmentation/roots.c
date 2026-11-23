/** @file
 * @brief Derive the exact common original root union from existing uncovered final regions.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * requires original analysis before edits, including newly exposed exclusions.
 */
#include "internal.h"
#include "scopes.h"
#include <string.h>
ProjectSourceAugmentationStatus project_source_augmentation_roots(ProjectSourceAugmentation *owner,
    const ProjectSourceRootSettings *settings, ProjectSourceAugmentationRoots *output) {
    const SourceRootCapture *configured = settings->configured;
    size_t original = configured->root_count, final = settings->final_count;
    if (original > SIZE_MAX - final || original + final > SIZE_MAX / sizeof(ModuleRoot)) { return owner->status = PROJECT_SOURCE_AUGMENTATION_LIMIT; }
    size_t capacity = original + final, count = original; bool augmented = false;
    ModuleRoot *roots = capacity ? source_arena_allocate(&owner->arena, capacity * sizeof(*roots)) : NULL;
    if (capacity && !roots) { return owner->status = PROJECT_SOURCE_AUGMENTATION_MEMORY; }
    for (size_t i = 0; i < original; i++) { roots[i] = configured->roots[i]; }
    for (size_t i = 0; i < final; i++) {
        const ModuleRoot *root = &settings->final[i];
        owner->region_status = source_operation_root_region(&owner->arena, root, configured->roots, original, &owner->failed_path);
        if (owner->region_status == SOURCE_OPERATION_ROOTS_OK) { continue; }
        if (owner->region_status != SOURCE_OPERATION_ROOTS_COVERAGE) { return owner->status = PROJECT_SOURCE_AUGMENTATION_REGION; }
        SourcePhysicalPath path;
        if (source_physical_destination(&owner->arena, root->path, SOURCE_PHYSICAL_DIRECTORY, &path, &owner->physical_issue) != SOURCE_PHYSICAL_OK) {
            owner->failed_path = owner->physical_issue.path; return owner->status = PROJECT_SOURCE_AUGMENTATION_PHYSICAL;
        }
        if (!path.present) { continue; }
        augmented = true; bool merged = false;
        for (size_t j = 0; j < count; j++) {
            if (strcmp(roots[j].path, root->path)) { continue; }
            owner->status = project_source_augmentation_scope_union(&owner->arena, &roots[j], root);
            if (owner->status != PROJECT_SOURCE_AUGMENTATION_OK) { return owner->status; }
            merged = true; break;
        }
        if (!merged) { roots[count++] = *root; }
    }
    *output = (ProjectSourceAugmentationRoots){roots, count, augmented}; return PROJECT_SOURCE_AUGMENTATION_OK;
}
