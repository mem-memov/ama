/** @file
 * @brief Capture actually present associated deletion material without executable-test requirements.
 * [TOOL-01](../../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * permits unfinished units with missing tests; [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * refuses observations contradicting the original captured source domain.
 */
#include <string.h>
#include "internal.h"
#include "../../capture/match.h"
/** Associated directories contain mains strictly below their slash boundary. */
static bool below(const char *path, const char *directory) {
    size_t length = strlen(directory);
    return !strncmp(path, directory, length) && path[length] == '/';
}
/** Even optional absence cannot hide an originally indexed test source. */
static bool observed(SourceRemovalSelection *selection, const char *directory, const AcquisitionUnitEntry *files) {
    for (const SourceCapturedUnit *unit = selection->tree->capture->units; unit; unit = unit->next) {
        const char *path = source_path(unit->source->text);
        if (!below(path, directory)) { continue; }
        if (!files || !source_captured_unit_matches(unit, files)) { selection->failed_path = path; return false; }
    }
    return true;
}
SourceRemovalSelectionStatus source_removal_selection_tests(SourceRemovalSelection *selection, const SourceCapturedUnit *unit) {
    AcquisitionTestDirectory *association = source_arena_allocate(&selection->arena, sizeof(*association));
    if (!association) { return SOURCE_REMOVAL_SELECTION_MEMORY; }
    selection->file_status = acquisition_test_directory(&selection->arena, unit->source, association);
    if (selection->file_status != SOURCE_OK) {
        return selection->file_status == SOURCE_OUT_OF_MEMORY || selection->file_status == SOURCE_LIMIT
            ? SOURCE_REMOVAL_SELECTION_MEMORY : SOURCE_REMOVAL_SELECTION_INVALID;
    }
    SourcePhysicalPath directory;
    SourcePhysicalStatus status = source_physical_resolve(&selection->arena, association->physical,
        SOURCE_PHYSICAL_DIRECTORY, false, &directory, &selection->physical_issue);
    if (status == SOURCE_PHYSICAL_MISSING) {
        return observed(selection, association->physical, NULL) ? SOURCE_REMOVAL_SELECTION_OK : SOURCE_REMOVAL_SELECTION_CHANGED;
    }
    if (status != SOURCE_PHYSICAL_OK) {
        selection->failed_path = selection->physical_issue.path;
        return status == SOURCE_PHYSICAL_STORAGE ? SOURCE_REMOVAL_SELECTION_MEMORY : SOURCE_REMOVAL_SELECTION_FILES;
    }
    const AcquisitionUnitEntry *files = NULL;
    selection->file_status = acquisition_directory_capture_root(&selection->arena, &selection->files,
        &directory, &files, &selection->failed_path);
    if (selection->file_status != SOURCE_OK) {
        return selection->file_status == SOURCE_OUT_OF_MEMORY || selection->file_status == SOURCE_LIMIT
            ? SOURCE_REMOVAL_SELECTION_MEMORY : SOURCE_REMOVAL_SELECTION_FILES;
    }
    if (!observed(selection, association->physical, files)) { return SOURCE_REMOVAL_SELECTION_CHANGED; }
    SourceRemovalGroup *group = source_arena_allocate(&selection->arena, sizeof(*group));
    if (!group) { return SOURCE_REMOVAL_SELECTION_MEMORY; }
    *group = (SourceRemovalGroup){.next = selection->groups, .files = files, .requester = unit,
        .association = association, .kind = SOURCE_REMOVAL_ASSOCIATED_TEST};
    selection->groups = group;
    return SOURCE_REMOVAL_SELECTION_OK;
}
