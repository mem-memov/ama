/** @file
 * @brief Merge exact removal observations and derive the complete original definition/file cuts.
 * [TOOL-03](../../../../../spec/implementation/tools-and-conformance.md#tool-03)
 * includes drafts and associated test helpers while preserving unrelated originals;
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * refuses contradictory repeated physical observations.
 */
#include <stdint.h>
#include <string.h>
#include "internal.h"
/** Physical-path equality alone cannot merge contradictory file/directory or byte observations. */
static bool equal(const AcquisitionUnitEntry *left, const AcquisitionUnitEntry *right) {
    if (left->directory != right->directory) { return false; }
    if (left->directory) { return true; }
    return source_length(left->file.content) == source_length(right->file.content)
        && !memcmp(source_bytes(left->file.content), source_bytes(right->file.content), source_length(left->file.content));
}
/** Keep one byte-sorted member per physical key while retaining all original groups separately. */
static SourceRemovalSelectionStatus insert(SourceRemovalSelection *selection, const AcquisitionUnitEntry *file, size_t *count) {
    SourceRemovalSelectedFile **position = &selection->selected;
    while (*position && strcmp((*position)->file->file.path, file->file.path) < 0) { position = &(*position)->next; }
    if (*position && !strcmp((*position)->file->file.path, file->file.path)) {
        if (equal((*position)->file, file)) { return SOURCE_REMOVAL_SELECTION_OK; }
        selection->failed_path = file->file.path;
        return SOURCE_REMOVAL_SELECTION_CHANGED;
    }
    if (*count == SIZE_MAX) { return SOURCE_REMOVAL_SELECTION_MEMORY; }
    SourceRemovalSelectedFile *entry = source_arena_allocate(&selection->arena, sizeof(*entry));
    if (!entry) { return SOURCE_REMOVAL_SELECTION_MEMORY; }
    *entry = (SourceRemovalSelectedFile){.next = *position, .file = file};
    *position = entry; (*count)++;
    return SOURCE_REMOVAL_SELECTION_OK;
}
/** Exact main membership determines removal; no ancestry or dependency expands this set. */
static bool contains(const SourceRemovalSelection *selection, const char *path) {
    for (const SourceRemovalSelectedFile *entry = selection->selected; entry; entry = entry->next) {
        int order = strcmp(entry->file->file.path, path);
        if (order == 0) { return true; }
        if (order > 0) { break; }
    }
    return false;
}
/** Emit original nominal identities in deterministic byte order, including inactive mains. */
static SourceRemovalSelectionStatus identities(SourceRemovalSelection *selection) {
    size_t count = 0;
    for (const SourceCapturedUnit *unit = selection->tree->capture->units; unit; unit = unit->next) {
        if (contains(selection, source_path(unit->source->text))) {
            if (count == SIZE_MAX) { return SOURCE_REMOVAL_SELECTION_MEMORY; }
            count++;
        }
    }
    if (count > SIZE_MAX / sizeof(const char *)) { return SOURCE_REMOVAL_SELECTION_MEMORY; }
    const char **names = count ? source_arena_allocate(&selection->arena, count * sizeof(*names)) : NULL;
    if (count && !names) { return SOURCE_REMOVAL_SELECTION_MEMORY; }
    size_t retained = 0;
    for (const SourceCapturedUnit *unit = selection->tree->capture->units; unit; unit = unit->next) {
        if (!contains(selection, source_path(unit->source->text))) { continue; }
        size_t index = retained++;
        while (index && strcmp(names[index - 1], unit->source->logical_path) > 0) { names[index] = names[index - 1]; index--; }
        names[index] = unit->source->logical_path;
    }
    selection->identities = names;
    selection->identity_count = count;
    return SOURCE_REMOVAL_SELECTION_OK;
}
SourceRemovalSelectionStatus source_removal_selection_index(SourceRemovalSelection *selection) {
    size_t count = 0;
    for (const SourceRemovalGroup *group = selection->groups; group; group = group->next) {
        for (const AcquisitionUnitEntry *file = group->files; file; file = file->next) {
            SourceRemovalSelectionStatus status = insert(selection, file, &count);
            if (status != SOURCE_REMOVAL_SELECTION_OK) { return status; }
        }
    }
    if (count > SIZE_MAX / sizeof(const char *)) { return SOURCE_REMOVAL_SELECTION_MEMORY; }
    const char **paths = count ? source_arena_allocate(&selection->arena, count * sizeof(*paths)) : NULL;
    if (count && !paths) { return SOURCE_REMOVAL_SELECTION_MEMORY; }
    size_t index = 0;
    for (const SourceRemovalSelectedFile *entry = selection->selected; entry; entry = entry->next) { paths[index++] = entry->file->file.path; }
    selection->paths = paths;
    selection->path_count = count;
    return identities(selection);
}
