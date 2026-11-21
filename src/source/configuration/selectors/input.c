/**
 * @file
 * @brief Resolve one complete package alias before reading untouched mask components.
 * [PKG-04](../../../../spec/project/packages.md#pkg-04) forbids module-local
 * aliases; [PKG-05](../../../../spec/project/packages.md#pkg-05) rejects traversal
 * and standard roots before matching, without filesystem effects.
 */
#include "internal.h"
#include "source/configuration/records/selector.h"

DependencySelectorStatus selector_input(SourceSpan expression, const ConfigurationAliasScope *package,
    SelectorCursor *output, SourceIssue *issue) {
    if (!configuration_selector(expression) || (package != NULL && package->kind != CONFIGURATION_PACKAGE_SCOPE)) {
        return selector_failure(issue, source_span_valid(expression) ? expression : (SourceSpan){0}, DEPENDENCY_SELECTOR_INVALID,
            "Invalid dependency selector spelling or scope", "an ordinary pattern using only package aliases");
    }
    const unsigned char *bytes = source_bytes(expression.source);
    SelectorCursor cursor = {.suffix = expression};
    if (bytes[expression.start] == '~') {
        size_t end = expression.start;
        while (end < expression.end && bytes[end] != '/') { end++; }
        SourceSpan key = {expression.source, expression.start, end};
        const ConfigurationAliasEntry *alias = configuration_alias_find(package, key);
        if (alias == NULL || alias->target->root != MODULE_ROOT_PROJECT) {
            return selector_failure(issue, key, DEPENDENCY_SELECTOR_INVALID,
                "Dependency alias is absent or targets the standard namespace", "one ordinary package alias");
        }
        cursor.base = alias->target;
        cursor.suffix.start = end == expression.end ? end : end + 1;
    } else { cursor.suffix.start++; }
    cursor.offset = cursor.suffix.start;
    *output = cursor;
    return DEPENDENCY_SELECTOR_OK;
}
