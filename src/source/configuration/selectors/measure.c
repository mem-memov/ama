/**
 * @file
 * @brief Measure complete expanded component storage without allocating or publishing.
 * [PKG-05](../../../../spec/project/packages.md#pkg-05) preserves every mask
 * component; [ARCH-05](../../../../spec/implementation/compiler.md#arch-05)
 * checks combined byte/count bounds before scalar storage preparation.
 */
#include "internal.h"

DependencySelectorStatus selector_measure(SelectorCursor cursor, SourceSpan expression,
    SelectorMeasure *output, SourceIssue *issue) {
    ModulePathPart part;
    size_t count = 0, bytes = 0;
    while (selector_next(&cursor, &part)) {
        if (count == SIZE_MAX || part.length > SIZE_MAX - bytes) {
            return selector_failure(issue, expression, DEPENDENCY_SELECTOR_LIMIT,
                "Dependency pattern size is not representable", "representable component and scalar counts");
        }
        count++;
        bytes += part.length;
    }
    if (count == 0) {
        return selector_failure(issue, expression, DEPENDENCY_SELECTOR_INVALID,
            "Dependency selector has no components", "a version or module pattern after alias expansion");
    }
    *output = (SelectorMeasure){count, bytes};
    return DEPENDENCY_SELECTOR_OK;
}
