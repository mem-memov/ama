/**
 * @file
 * @brief Orchestrate admitted input, exact sizing and immutable selector publication.
 * [PKG-05](../../../../spec/project/packages.md#pkg-05) separates masks from
 * real-path normalization. [ARCH-03](../../../../spec/implementation/compiler.md#arch-03)
 * retains failed preparation without publishing partial scalar arrays.
 */
#include "internal.h"

DependencySelectorStatus dependency_selector_compile(SourceArena *arena, SourceSpan expression,
    const ConfigurationAliasScope *package, const DependencySelector **output, SourceIssue *issue) {
    SelectorCursor input;
    DependencySelectorStatus status = selector_input(expression, package, &input, issue);
    if (status != DEPENDENCY_SELECTOR_OK) { return status; }
    SelectorMeasure measure;
    status = selector_measure(input, expression, &measure, issue);
    if (status != DEPENDENCY_SELECTOR_OK) { return status; }
    return selector_store(arena, input, measure, expression, output, issue);
}
