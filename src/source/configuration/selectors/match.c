/**
 * @file
 * @brief Compose eligible identity admission and finite component-prefix reachability.
 * [PKG-05](../../../../spec/project/packages.md#pkg-05) restricts candidates to
 * complete ordinary versions/modules and unions all nonempty chain partitions.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) retires matching
 * scratch on every exit without publishing an answer after resource failure.
 */
#include <string.h>
#include "internal.h"
#include "source/path/logical.h"

/** Path DP cell (i,j) means the first i patterns match exactly j whole components.
 * [PKG-05](../../../../spec/project/packages.md#pkg-05) gives a whole-star its
 * first component from the previous row and further components from this row.
 */
static bool reachability(const DependencySelector *selector, size_t count, SelectorWorkspace workspace) {
    size_t capacity = count + 1;
    bool *previous = workspace.path, *current = workspace.path + capacity;
    memset(previous, 0, capacity * sizeof(bool));
    previous[0] = true;
    for (size_t index = 0; index < selector->count; index++) {
        DependencySelectorPart part = selector->parts[index];
        current[0] = false;
        for (size_t column = 1; column <= count; column++) {
            current[column] = part.chain ? previous[column - 1] || current[column - 1]
                : previous[column - 1] && selector_word_match(part, workspace.words[column - 1],
                    workspace.word, workspace.word_capacity);
        }
        bool *swap = previous;
        previous = current;
        current = swap;
    }
    return previous[count];
}

DependencySelectorStatus dependency_selector_match(const DependencySelector *selector,
    const unsigned char *bytes, size_t length, SourceAllocator allocator, bool *output, SourceIssue *issue) {
    ModulePathShape shape;
    if (!module_path_describe(bytes, length, &shape) || shape.root != MODULE_ROOT_PROJECT
        || (shape.extent != MODULE_PATH_VERSION && shape.extent != MODULE_PATH_MODULE)) {
        *output = false;
        return DEPENDENCY_SELECTOR_OK;
    }
    SourceArena scratch;
    source_arena_init(&scratch, allocator);
    SelectorWorkspace workspace;
    DependencySelectorStatus status = selector_workspace(&scratch, bytes, length, shape.components,
        selector->expression, &workspace, issue);
    if (status == DEPENDENCY_SELECTOR_OK) { *output = reachability(selector, shape.components, workspace); }
    source_arena_destroy(&scratch);
    return status;
}
