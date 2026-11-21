/**
 * @file
 * @brief Form a sparse union of actual selected roots without retaining masks or duplicating the view.
 * [PKG-05](../../../../spec/project/packages.md#pkg-05) selects every matching
 * identity once. [ARCH-03](../../../../spec/implementation/compiler.md#arch-03)
 * retains partial root storage on refusal and publishes only a complete union.
 */
#include "internal.h"

DependencySelectionStatus dependency_selection_build(SourceArena *arena, const DependencyView *view,
    const ModulePathValue *owner, const DependencySelector *const *patterns, size_t count,
    const DependencySelection **output, SourceIssue *issue) {
    const DependencyViewEntry *version = owner == NULL ? NULL : selection_find(view, owner->text, owner->length);
    if (version == NULL || version->shape.extent != MODULE_PATH_VERSION || (count != 0 && patterns == NULL)) {
        return selection_failure(issue, owner == NULL ? (SourceSpan){0} : owner->expression,
            DEPENDENCY_SELECTION_INVALID, "Selection has no valid declaring version or pattern array",
            "an actual ordinary version and complete compiled-selector array");
    }
    DependencySelection *selection = source_arena_allocate(arena, sizeof(*selection));
    if (selection == NULL) {
        return selection_failure(issue, owner->expression, DEPENDENCY_SELECTION_OUT_OF_MEMORY,
            "Dependency selection allocation failed", "available selection storage");
    }
    *selection = (DependencySelection){.view = view, .owner = version};
    DependencySelectionRoot *last = NULL;
    for (size_t index = 0; index < view->count; index++) {
        const DependencyViewEntry *entry = &view->entries[index];
        for (size_t pattern = 0; pattern < count; pattern++) {
            bool matched = false;
            DependencySelectorStatus status = dependency_selector_match(patterns[pattern],
                (const unsigned char *)entry->identity->text, entry->identity->length,
                arena->allocator, &matched, issue);
            if (status != DEPENDENCY_SELECTOR_OK) {
                return status == DEPENDENCY_SELECTOR_OUT_OF_MEMORY ? DEPENDENCY_SELECTION_OUT_OF_MEMORY
                    : status == DEPENDENCY_SELECTOR_LIMIT ? DEPENDENCY_SELECTION_LIMIT : DEPENDENCY_SELECTION_INVALID;
            }
            if (!matched) { continue; }
            DependencySelectionRoot *root = source_arena_allocate(arena, sizeof(*root));
            if (root == NULL) {
                return selection_failure(issue, patterns[pattern]->expression, DEPENDENCY_SELECTION_OUT_OF_MEMORY,
                    "Selected dependency root allocation failed", "available selected-root storage");
            }
            *root = (DependencySelectionRoot){entry, NULL};
            if (last == NULL) { selection->roots = root; }
            else { last->next = root; }
            last = root;
            selection->count++;
            break;
        }
    }
    *output = selection;
    return DEPENDENCY_SELECTION_OK;
}
