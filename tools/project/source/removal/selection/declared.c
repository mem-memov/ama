/** @file
 * @brief Retain explicitly required non-module removal inputs without guessing resource ownership.
 * [PKG-07](../../../../../spec/project/packages.md#pkg-07) requires explicit
 * asset declarations; [TOOL-03](../../../../../spec/implementation/tools-and-conformance.md#tool-03)
 * preserves shared surviving consumers; data roles do not conceal removed module vertices.
 */
#include <string.h>
#include "internal.h"
/** Only actual raw/test main membership selects a declared group's requester. */
bool source_removal_selection_contains_main(const SourceRemovalSelection *selection, const SourceCapturedUnit *requester) {
    const char *path = source_path(requester->source->text);
    for (const SourceRemovalGroup *group = selection->groups; group; group = group->next) {
        for (const AcquisitionUnitEntry *file = group->files; file; file = file->next) {
            if (!strcmp(file->file.path, path)) { return true; }
        }
    }
    return false;
}
SourceRemovalSelectionStatus source_removal_selection_declared(SourceRemovalSelection *selection,
    const SourceDeclaredGroup *declared, const SourceCapturedUnit *requester) {
    for (const SourceDeclaredGroup *input = declared; input; input = input->next) {
        if (input->requester != requester) { continue; }
        SourceRemovalGroup *group = source_arena_allocate(&selection->arena, sizeof(*group));
        if (!group) { return SOURCE_REMOVAL_SELECTION_MEMORY; }
        *group = (SourceRemovalGroup){.next = selection->groups, .files = input->files,
            .requester = requester, .kind = SOURCE_REMOVAL_DECLARED_INPUT};
        selection->groups = group;
    }
    return SOURCE_REMOVAL_SELECTION_OK;
}
