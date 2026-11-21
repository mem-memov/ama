/**
 * @file
 * @brief Establish exact deterministic alias order and reject all shared/local collisions.
 * [PKG-04](../../../../spec/project/packages.md#pkg-04) forbids shadowing and
 * declaration-order binding. Insertion sorting is finite and explicitly O(n²)
 * comparisons rather than an unproved claim about a host sorting implementation.
 */
#include <string.h>
#include "internal.h"

int configuration_scope_compare(SourceSpan left, SourceSpan right) {
    size_t a = left.end - left.start;
    size_t b = right.end - right.start;
    size_t common = a < b ? a : b;
    int comparison = memcmp(source_bytes(left.source) + left.start, source_bytes(right.source) + right.start, common);
    if (comparison != 0) { return comparison; }
    return a < b ? -1 : a > b ? 1 : 0;
}

ConfigurationScopeStatus configuration_scope_keys(ConfigurationScopeWork work,
    const ConfigurationAliasScope *package, SourceIssue *issue) {
    for (size_t index = 1; index < work.scope->count; index++) {
        ConfigurationAliasEntry entry = work.entries[index];
        size_t cursor = index;
        while (cursor != 0 && configuration_scope_compare(work.entries[cursor - 1].name, entry.name) > 0) {
            work.entries[cursor] = work.entries[cursor - 1];
            cursor--;
        }
        if (cursor != 0 && configuration_scope_compare(work.entries[cursor - 1].name, entry.name) == 0) {
            return configuration_scope_failure(issue, entry.name, CONFIGURATION_SCOPE_INVALID,
                "Duplicate alias in scope preparation", "one declaration per exact alias name");
        }
        work.entries[cursor] = entry;
    }
    for (size_t index = 0; package != NULL && index < work.scope->count; index++) {
        if (configuration_scope_own(package, work.entries[index].name) != NULL) {
            return configuration_scope_failure(issue, work.entries[index].name, CONFIGURATION_SCOPE_INVALID,
                "Local alias collides with a package alias", "distinct shared and local alias names");
        }
    }
    return CONFIGURATION_SCOPE_OK;
}
