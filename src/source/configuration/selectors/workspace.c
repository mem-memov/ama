/**
 * @file
 * @brief Allocate bounded component views and reusable two-row match workspaces.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) gives the
 * orchestrator one local scratch owner; [ARCH-05](../../../../spec/implementation/compiler.md#arch-05)
 * checks all array and row sums. [PKG-05](../../../../spec/project/packages.md#pkg-05)
 * keeps candidate separators outside scalar matching.
 */
#include "internal.h"

DependencySelectorStatus selector_workspace(SourceArena *arena, const unsigned char *bytes, size_t length,
    size_t count, SourceSpan expression, SelectorWorkspace *output, SourceIssue *issue) {
    size_t maximum = 0, index = 0;
    if (count > SIZE_MAX / sizeof(SelectorWord)) { goto limit; }
    for (size_t cursor = 1; cursor < length;) {
        size_t end = cursor;
        while (end < length && bytes[end] != '/') { end++; }
        if (end - cursor > maximum) { maximum = end - cursor; }
        cursor = end == length ? end : end + 1;
    }
    if (count == SIZE_MAX || maximum == SIZE_MAX) { goto limit; }
    size_t path = count + 1, word = maximum + 1;
    if (path > SIZE_MAX / 2 || word > SIZE_MAX / 2 || 2 * path > SIZE_MAX - 2 * word
        || 2 * path + 2 * word > SIZE_MAX / sizeof(bool)) { goto limit; }
    SelectorWord *words = source_arena_allocate(arena, count * sizeof(*words));
    bool *rows = words == NULL ? NULL : source_arena_allocate(arena, (2 * path + 2 * word) * sizeof(bool));
    if (rows == NULL) {
        return selector_failure(issue, expression, DEPENDENCY_SELECTOR_OUT_OF_MEMORY,
            "Dependency match allocation failed", "available local component and DP rows");
    }
    for (size_t cursor = 1; cursor < length;) {
        size_t end = cursor;
        while (end < length && bytes[end] != '/') { end++; }
        words[index++] = (SelectorWord){bytes + cursor, end - cursor};
        cursor = end == length ? end : end + 1;
    }
    *output = (SelectorWorkspace){words, rows, rows + 2 * path, word};
    return DEPENDENCY_SELECTOR_OK;
limit:
    return selector_failure(issue, expression, DEPENDENCY_SELECTOR_LIMIT,
        "Dependency match size is not representable", "representable component and DP row storage");
}
