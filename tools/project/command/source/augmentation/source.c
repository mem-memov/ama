/** @file
 * @brief Compare both raw membership inclusions and actual endpoint identity after common original augmentation.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * rejects contradictory captured facts without a recapture fallback.
 */
#include "internal.h"
#include <string.h>
/** Both inclusion directions compare original physical facts, not merely names
 * or a new selected-unit count; opaque material belongs to the raw contract too. */
static bool contains(const AcquisitionUnitEntry *entries, const AcquisitionUnitEntry *wanted) {
    for (const AcquisitionUnitEntry *entry = entries; entry; entry = entry->next) {
        if (strcmp(entry->file.path, wanted->file.path)) { continue; }
        if (entry->directory != wanted->directory || entry->file.state != wanted->file.state) { return false; }
        if (entry->directory) { return true; }
        return entry->file.content && wanted->file.content && source_length(entry->file.content) == source_length(wanted->file.content)
            && !memcmp(source_bytes(entry->file.content), source_bytes(wanted->file.content), source_length(wanted->file.content));
    }
    return false;
}
ProjectSourceAugmentationStatus project_source_augmentation_source(ProjectSourceAugmentation *owner,
    const ProjectSourceRootSettings *settings) {
    if (source_tree_selection_read(&owner->source, &owner->capture, settings->placement->selection->operand.path) != SOURCE_TREE_OK) {
        owner->failed_path = owner->source.failed_path; return owner->status = PROJECT_SOURCE_AUGMENTATION_SELECTION;
    }
    const struct stat *before = &settings->placement->selection->operand.metadata, *after = &owner->source.operand.metadata;
    if (before->st_dev != after->st_dev || before->st_ino != after->st_ino || before->st_mode != after->st_mode) {
        owner->failed_path = owner->source.operand.path; return owner->status = PROJECT_SOURCE_AUGMENTATION_CHANGED;
    }
    const AcquisitionUnitEntry *old = settings->placement->selection->members, *now = owner->source.members;
    for (unsigned side = 0; side < 2; side++) {
        for (const AcquisitionUnitEntry *entry = side ? now : old; entry; entry = entry->next) {
            if (!contains(side ? old : now, entry)) { owner->failed_path = entry->file.path; return owner->status = PROJECT_SOURCE_AUGMENTATION_CHANGED; }
        }
    }
    return PROJECT_SOURCE_AUGMENTATION_OK;
}
