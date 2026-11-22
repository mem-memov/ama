/** @file
 * @brief Map only proven original roots and intersect their names with actual final availability.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * forbids silently expanding an old wildcard; [PKG-05](../../../../../spec/project/packages.md#pkg-05)
 * permits empty matches and preserves normal selected unit/version coverage.
 */
#include "roots.h"
#include <stdint.h>
#include <string.h>
/** Pointer membership validates real view authority before dereferencing a pair. */
static bool member(const DependencyView *view, const DependencyViewEntry *entry) {
    for (size_t i = 0; i < view->count; i++) { if (entry == &view->entries[i]) { return true; } }
    return false;
}
/** Version-root mapping is explicit; module-root mapping uses exact C identities. */
static const char *mapped(const DependencyViewEntry *entry, const SourceCloneMap *map,
    const SourceSelectionVersionPair *pairs, size_t count) {
    if (entry->shape.extent == MODULE_PATH_VERSION) {
        for (size_t i = 0; i < count; i++) { if (pairs[i].original == entry) { return pairs[i].final->identity->text; } }
    } else {
        for (const SourceCloneMapping *row = map->first; row; row = row->next) {
            if (!strcmp(row->original->logical_path, entry->identity->text)) { return row->identity; }
        }
    }
    return entry->identity->text;
}
/** Test set membership by names, never by a new mask evaluation or byte prefix. */
static bool requested(const DependencySelection *original, const DependencyViewEntry *entry,
    const SourceCloneMap *map, bool copied, const SourceSelectionVersionPair *pairs, size_t count) {
    for (const DependencySelectionRoot *root = original->roots; root; root = root->next) {
        const char *name = copied ? mapped(root->entry, map, pairs, count) : root->entry->identity->text;
        if (!strcmp(name, entry->identity->text)) { return true; }
    }
    return false;
}
SourceSelectionRootsStatus source_copy_selected_roots(SourceArena *arena, const DependencySelection *original,
    const DependencyView *final, const SourceCloneMap *map, bool copied,
    const SourceSelectionVersionPair *pairs, size_t count, SourceSelectionRoots *output, SourceSelectionRootsIssue *issue) {
    if (!issue) { return SOURCE_SELECTION_ROOTS_INVALID; } *issue = (SourceSelectionRootsIssue){0};
    if (!arena || !original || !original->view || !final || !output || (count && !pairs)
        || (copied ? !map || !map->ready : map != NULL || count != 0)) { return SOURCE_SELECTION_ROOTS_INVALID; }
    for (size_t i = 0; i < count; i++) {
        if (!member(original->view, pairs[i].original) || !member(final, pairs[i].final)
            || pairs[i].original->shape.extent != MODULE_PATH_VERSION || pairs[i].final->shape.extent != MODULE_PATH_VERSION) {
            return SOURCE_SELECTION_ROOTS_INVALID;
        }
        for (size_t j = 0; j < i; j++) {
            if (pairs[j].original == pairs[i].original && pairs[j].final != pairs[i].final) {
                *issue = (SourceSelectionRootsIssue){&pairs[j], &pairs[i]}; return SOURCE_SELECTION_ROOTS_CONFLICT;
            }
        }
    }
    size_t used = 0;
    for (size_t i = 0; i < final->count; i++) { if (requested(original, &final->entries[i], map, copied, pairs, count)) { used++; } }
    if (used > SIZE_MAX / sizeof(DependencyViewEntry *)) { return SOURCE_SELECTION_ROOTS_LIMIT; }
    const DependencyViewEntry **entries = used ? source_arena_allocate(arena, used * sizeof(*entries)) : NULL;
    if (used && !entries) { return SOURCE_SELECTION_ROOTS_MEMORY; }
    size_t next = 0;
    for (size_t i = 0; i < final->count; i++) {
        if (requested(original, &final->entries[i], map, copied, pairs, count)) { entries[next++] = &final->entries[i]; }
    }
    *output = (SourceSelectionRoots){entries, used}; return SOURCE_SELECTION_ROOTS_OK;
}
