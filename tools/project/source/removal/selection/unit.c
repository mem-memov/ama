/** @file
 * @brief Complete every physically selected original module's full deletion unit.
 * [PKG-07](../../../../../spec/project/packages.md#pkg-07) retains complete
 * companions/subtrees regardless of selection cause; [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * rejects contradictory original observations without replacing the snapshot.
 */
#include <string.h>
#include "internal.h"
#include "../../capture/match.h"
SourceRemovalSelectionStatus source_removal_selection_unit(SourceRemovalSelection *selection, const SourceCapturedUnit *unit) {
    const char *path = source_path(unit->source->text), *slash = strrchr(path, '/');
    if (!slash) { return SOURCE_REMOVAL_SELECTION_INVALID; }
    char *parent = source_arena_copy(&selection->arena, path);
    if (!parent) { return SOURCE_REMOVAL_SELECTION_MEMORY; }
    parent[slash - path] = 0;
    const AcquisitionUnitEntry *files = NULL;
    selection->file_status = acquisition_unit_capture(&selection->arena, &selection->files,
        *parent ? parent : "/", slash + 1, ACQUISITION_ACTIVE, &files, &selection->failed_path);
    if (selection->file_status != SOURCE_OK) {
        return selection->file_status == SOURCE_OUT_OF_MEMORY || selection->file_status == SOURCE_LIMIT
            ? SOURCE_REMOVAL_SELECTION_MEMORY : SOURCE_REMOVAL_SELECTION_FILES;
    }
    size_t length = strlen(path);
    for (const SourceCapturedUnit *original = selection->tree->capture->units; original; original = original->next) {
        const char *candidate = source_path(original->source->text);
        bool covered = !strcmp(path, candidate) || (!strncmp(path, candidate, length)
            && candidate[length] == '+' && candidate[length + 1] == '/');
        if (covered && !source_captured_unit_matches(original, files)) {
            selection->failed_path = candidate;
            return SOURCE_REMOVAL_SELECTION_CHANGED;
        }
    }
    SourceRemovalGroup *group = source_arena_allocate(&selection->arena, sizeof(*group));
    if (!group) { return SOURCE_REMOVAL_SELECTION_MEMORY; }
    *group = (SourceRemovalGroup){.next = selection->groups, .files = files,
        .requester = unit, .kind = SOURCE_REMOVAL_UNIT};
    selection->groups = group;
    return SOURCE_REMOVAL_SELECTION_OK;
}
