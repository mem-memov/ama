/** @file
 * @brief Compute the unique minimal ancestor cover of all exact before/after material changes.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires one complete staged replacement per disjoint mutation, never partial merging.
 */
#include <stdint.h>
#include <string.h>
#include "internal.h"
/** Changed candidate has no changed strict ancestor iff it is a mutation root. */
static bool minimal(const char *path, const char *const *candidates, size_t count) {
    for (size_t i = 0; i < count; i++) {
        if (strcmp(path, candidates[i]) && source_image_below(path, candidates[i])) { return false; }
    }
    return true;
}
SourceForestStatus source_mutation_forest(SourceArena *arena, const SourceCatalogImage *before, const SourceCatalogImage *after,
    SourceMutationForest *output, SourceForestIssue *issue) {
    if (!issue) { return SOURCE_FOREST_INVALID; } *issue = (SourceForestIssue){0};
    if (!arena || !before || !before->valid || !after || !after->valid || !output || !source_image_same_bounds(before, after)) { return SOURCE_FOREST_INVALID; }
    if (before->count > SIZE_MAX - after->count || before->count + after->count > SIZE_MAX / sizeof(char *)) { return SOURCE_FOREST_LIMIT; }
    size_t capacity = before->count + after->count;
    const char **candidates = capacity ? source_arena_allocate(arena, capacity * sizeof(*candidates)) : NULL;
    if (capacity && !candidates) { return SOURCE_FOREST_MEMORY; }
    size_t count = 0;
    for (size_t i = 0; i < before->count; i++) {
        const SourceCatalogImageEntry *old = &before->entries[i];
        if (!source_forest_equal(old, source_forest_find(after, old->path))) { candidates[count++] = old->path; }
    }
    for (size_t i = 0; i < after->count; i++) {
        if (!source_forest_find(before, after->entries[i].path)) { candidates[count++] = after->entries[i].path; }
    }
    size_t roots = 0;
    for (size_t i = 0; i < count; i++) { if (minimal(candidates[i], candidates, count)) { roots++; } }
    if (roots > SIZE_MAX / sizeof(SourceMutationRoot)) { return SOURCE_FOREST_LIMIT; }
    SourceMutationRoot *rows = roots ? source_arena_allocate(arena, roots * sizeof(*rows)) : NULL;
    if (roots && !rows) { return SOURCE_FOREST_MEMORY; }
    size_t used = 0;
    for (size_t i = 0; i < count; i++) {
        if (!minimal(candidates[i], candidates, count)) { continue; }
        size_t index = used++;
        while (index && strcmp(rows[index - 1].path, candidates[i]) > 0) { rows[index] = rows[index - 1]; index--; }
        rows[index] = (SourceMutationRoot){.path = candidates[i], .before = source_forest_find(before, candidates[i]), .after = source_forest_find(after, candidates[i])};
    }
    for (size_t i = 0; i < roots; i++) {
        SourceForestStatus status = source_forest_parent(arena, before, after, &rows[i], issue);
        if (status == SOURCE_FOREST_OK) { status = source_forest_material(arena, before, rows[i].path, &rows[i].before_entries, &rows[i].before_count, issue); }
        if (status == SOURCE_FOREST_OK) { status = source_forest_material(arena, after, rows[i].path, &rows[i].after_entries, &rows[i].after_count, issue); }
        if (status != SOURCE_FOREST_OK) { return status; }
        if ((!rows[i].before && rows[i].before_count) || (!rows[i].after && rows[i].after_count)) { return SOURCE_FOREST_INVALID; }
    }
    *output = (SourceMutationForest){rows, roots}; *issue = (SourceForestIssue){0}; return SOURCE_FOREST_OK;
}
