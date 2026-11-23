/** @file
 * @brief Join original removal selections before the single survivor-cut review.
 * [TOOL-03](../../../../../spec/implementation/tools-and-conformance.md#tool-03)
 * checks move-source and replacement removals together: an owner removed by either
 * selection is not a surviving consumer. Final replacement availability grants
 * no permission to erase an original outside dependency.
 */
#include <string.h>
#include "internal.h"
/** Equal occurrence provenance is one original requirement, even when both
 * selections independently captured its source bytes in different text owners. */
static bool same_use(const SourceRemovalInputUse *left, const SourceRemovalInputUse *right) {
    if (left->owner != right->owner || strcmp(left->path, right->path)
        || left->origin.start != right->origin.start || left->origin.end != right->origin.end) { return false; }
    if (left->origin.source == right->origin.source) { return true; }
    if (!left->origin.source || !right->origin.source) { return false; }
    return !strcmp(source_path(left->origin.source), source_path(right->origin.source))
        && source_length(left->origin.source) == source_length(right->origin.source)
        && !memcmp(source_bytes(left->origin.source), source_bytes(right->origin.source), source_length(left->origin.source));
}
SourceRemovalSelectionStatus source_removal_selection_merge(SourceRemovalSelection *output,
    const SourceRemovalSelection *const *parts, size_t count) {
    if (!output || output->attempted || !parts || !count) { return SOURCE_REMOVAL_SELECTION_INVALID; }
    for (size_t index = 0; index < count; index++) {
        if (!parts[index] || parts[index] == output || !parts[index]->valid
            || !parts[index]->tree || !parts[index]->analysis
            || parts[index]->tree->capture != parts[0]->tree->capture
            || parts[index]->analysis != parts[0]->analysis) { return SOURCE_REMOVAL_SELECTION_INVALID; }
    }
    output->attempted = true; output->tree = parts[0]->tree; output->analysis = parts[0]->analysis;
    SourceRemovalGroup **groups = &output->groups;
    SourceRemovalInputUse **uses = &output->inputs;
    for (size_t index = 0; index < count; index++) {
        for (const SourceRemovalGroup *group = parts[index]->groups; group; group = group->next) {
            SourceRemovalGroup *copy = source_arena_allocate(&output->arena, sizeof(*copy));
            if (!copy) { return output->status = SOURCE_REMOVAL_SELECTION_MEMORY; }
            *copy = *group; copy->next = NULL; *groups = copy; groups = &copy->next;
        }
        for (const SourceRemovalInputUse *use = parts[index]->inputs; use; use = use->next) {
            const SourceRemovalInputUse *found = output->inputs;
            while (found && !same_use(found, use)) { found = found->next; }
            if (found) { continue; }
            SourceRemovalInputUse *copy = source_arena_allocate(&output->arena, sizeof(*copy));
            if (!copy) { return output->status = SOURCE_REMOVAL_SELECTION_MEMORY; }
            *copy = *use; copy->next = NULL; *uses = copy; uses = &copy->next;
        }
    }
    output->status = source_removal_selection_index(output);
    output->valid = output->status == SOURCE_REMOVAL_SELECTION_OK;
    return output->status;
}
