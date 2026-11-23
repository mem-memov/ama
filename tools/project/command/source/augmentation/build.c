/** @file
 * @brief Select uncovered existing root regions exactly and capture one augmented original union.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * forbids missing final-root inputs and separately merged original registries.
 */
#include "internal.h"
ProjectSourceAugmentationStatus project_source_augmentation_build(ProjectSourceAugmentation *owner, const ProjectSourceRootSettings *settings) {
    if (!owner || owner->attempted) { return PROJECT_SOURCE_AUGMENTATION_INVALID; }
    owner->attempted = true;
    if (!settings || !settings->valid) { return owner->status = PROJECT_SOURCE_AUGMENTATION_INVALID; }
    ProjectSourceAugmentationRoots roots;
    owner->status = project_source_augmentation_roots(owner, settings, &roots);
    if (owner->status != PROJECT_SOURCE_AUGMENTATION_OK) { return owner->status; }
    if (!roots.expanded) { owner->original = settings->configured; owner->selected = settings->placement->selection; }
    else {
        if (source_root_capture_build(&owner->capture, roots.values, roots.count) != SOURCE_CAPTURE_OK) {
            owner->failed_path = owner->capture.failed_path; return owner->status = PROJECT_SOURCE_AUGMENTATION_CAPTURE;
        }
        owner->status = project_source_augmentation_source(owner, settings);
        if (owner->status != PROJECT_SOURCE_AUGMENTATION_OK) { return owner->status; }
        owner->original = &owner->capture; owner->selected = &owner->source;
    }
    owner->failed_path = NULL; owner->valid = true; return owner->status = PROJECT_SOURCE_AUGMENTATION_OK;
}
