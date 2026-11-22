/** @file
 * @brief Compose exact captured main placement, root decoding and injective clone admission.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * defines simultaneous original-to-copy correspondence; helpers here never infer
 * that correspondence by rewriting an original logical name.
 */
#include <string.h>
#include "propose.h"
/** A selected indexed main must have exactly its captured regular-file member. */
static const SourceTreePlacementEntry *main_entry(const SourceTreePlacement *placement,
    const SourceCapturedUnit *unit) {
    const char *path = source_path(unit->source->text);
    for (const SourceTreePlacementEntry *entry = placement->entries; entry; entry = entry->next) {
        if (!entry->original->directory && !strcmp(entry->original->file.path, path)) { return entry; }
    }
    return NULL;
}
SourceCloneStatus source_clone_map_propose_tree(SourceCloneMap *map, const SourceTreePlacement *placement,
    const ModuleRoot *roots, size_t count, SourceCloneTreeIssue *issue) {
    if (!map || map->ready) { return SOURCE_CLONE_INVALID; }
    if (map->status != SOURCE_CLONE_OK) { return map->status; }
    if (!issue || !placement || !placement->ready || !placement->selection
        || placement->selection->capture != map->capture || (count && !roots)) {
        return map->status = SOURCE_CLONE_INVALID;
    }
    *issue = (SourceCloneTreeIssue){0};
    for (size_t index = 0; index < placement->selection->count; index++) {
        const SourceCapturedUnit *unit = placement->selection->units[index];
        const SourceTreePlacementEntry *entry = main_entry(placement, unit);
        issue->unit = unit; issue->path = entry ? entry->path : source_path(unit->source->text);
        if (!entry) { return map->status = SOURCE_CLONE_INVALID; }
        SourcePlacedIdentity identity = {0};
        issue->identity_status = source_placement_identity(&map->arena, roots, count, entry->path, &identity);
        if (issue->identity_status != SOURCE_IDENTITY_OK) {
            return map->status = issue->identity_status == SOURCE_IDENTITY_MEMORY
                ? SOURCE_CLONE_MEMORY : SOURCE_CLONE_INVALID;
        }
        const SourceCloneMapping *row = NULL;
        SourceCloneStatus status = source_clone_map_add(map, unit->source, entry->path, identity.identity, &row);
        if (status != SOURCE_CLONE_OK) { return status; }
    }
    *issue = (SourceCloneTreeIssue){0};
    return SOURCE_CLONE_OK;
}
