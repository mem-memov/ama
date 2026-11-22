/** @file
 * @brief Compose preserved root images, fixed-baseline permission checks and exact INI edits.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * permits exact set preservation for shared environments without expanding masks
 * or editing their alias/repository tables.
 */
#include "prepare.h"
#include <stdint.h>
#include <string.h>
/** Scope identity is authority; byte-equal foreign rows are not owned scopes. */
static bool owns(const SourcePackageEnvironment *environment, const SourcePackageScope *scope) {
    if (!environment || !environment->valid) { return false; }
    for (const SourcePackageScope *row = environment->first; row; row = row->next) { if (row == scope) { return true; } }
    return false;
}
/** Both child arrays use actual view pointers, so membership needs no alias or
 * name interpretation and cannot merge distinct entries with similar names. */
static bool contains(const SourceSelectionRoots *roots, const DependencyViewEntry *entry) {
    for (size_t i = 0; i < roots->count; i++) { if (roots->entries[i] == entry) { return true; } }
    return false;
}
SourceConfigurationStatus source_copy_configuration_prepare(SourceArena *arena, SourceTextEdits *edits,
    const SourceCopyClosure *copy, const SourceCloneMap *map,
    const SourcePackageEnvironment *original, const SourcePackageScope *before,
    const SourcePackageEnvironment *final, const SourcePackageScope *after,
    bool copied, const SourceSelectionVersionPair *pairs, size_t pair_count,
    SourceSelectionRoots *output, SourceConfigurationIssue *issue) {
    if (!issue) { return SOURCE_CONFIGURATION_INVALID; } *issue = (SourceConfigurationIssue){0};
    if (!arena || !edits || edits->attempted || edits->status != SOURCE_EDITS_OK || !output
        || !copy || !copy->valid || !owns(original, before) || !owns(final, after)
        || original->catalog != &copy->tree->capture->catalog
        || edits->original != before->document.source || !map || !map->ready
        || map->capture != copy->tree->capture
        || (!copied && (pairs || pair_count || strcmp(before->version->logical_path, after->version->logical_path)))) { return SOURCE_CONFIGURATION_INVALID; }
    SourceSelectionRoots baseline = {0}, additions = {0};
    issue->roots_status = source_copy_selected_roots(arena, before->selection, final->view,
        copied ? map : NULL, copied, pairs, pair_count, &baseline, &issue->roots);
    if (issue->roots_status != SOURCE_SELECTION_ROOTS_OK) { return SOURCE_CONFIGURATION_ROOTS; }
    issue->permission_status = source_copy_permission_additions(arena, copy, map, final, after,
        &baseline, &additions, &issue->permission);
    if (issue->permission_status != SOURCE_PERMISSION_OK) { return SOURCE_CONFIGURATION_PERMISSIONS; }
    size_t count = 0;
    for (size_t i = 0; i < final->view->count; i++) {
        const DependencyViewEntry *entry = &final->view->entries[i];
        if (contains(&baseline, entry) || contains(&additions, entry)) { count++; }
    }
    if (count > SIZE_MAX / sizeof(DependencyViewEntry *)) { return SOURCE_CONFIGURATION_LIMIT; }
    const DependencyViewEntry **entries = count ? source_arena_allocate(arena, count * sizeof(*entries)) : NULL;
    if (count && !entries) { return SOURCE_CONFIGURATION_MEMORY; }
    size_t next = 0;
    for (size_t i = 0; i < final->view->count; i++) {
        const DependencyViewEntry *entry = &final->view->entries[i];
        if (contains(&baseline, entry) || contains(&additions, entry)) { entries[next++] = entry; }
    }
    issue->edits_status = source_copy_reexpress_selectors(edits, &before->document, final->view, entries, count);
    if (issue->edits_status != SOURCE_EDITS_OK) { return SOURCE_CONFIGURATION_EDITS; }
    *output = (SourceSelectionRoots){entries, count}; return SOURCE_CONFIGURATION_OK;
}
