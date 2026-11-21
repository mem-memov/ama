/**
 * @file
 * @brief Search at most two immutable exact-name tables without recursive scope traversal.
 * [PKG-04](../../../../spec/project/packages.md#pkg-04) permits only own aliases
 * plus one explicit package parent, and never a shorter alias-name fallback.
 */
#include "internal.h"

const ConfigurationAliasEntry *configuration_scope_own(const ConfigurationAliasScope *scope, SourceSpan key) {
    if (scope == NULL || !source_span_valid(key) || key.start == key.end) { return NULL; }
    size_t low = 0;
    size_t high = scope->count;
    while (low < high) {
        size_t middle = low + (high - low) / 2;
        int comparison = configuration_scope_compare(scope->entries[middle].name, key);
        if (comparison == 0) { return &scope->entries[middle]; }
        if (comparison < 0) { low = middle + 1; }
        else { high = middle; }
    }
    return NULL;
}

const ConfigurationAliasEntry *configuration_alias_find(const ConfigurationAliasScope *scope, SourceSpan key) {
    const ConfigurationAliasEntry *entry = configuration_scope_own(scope, key);
    if (entry != NULL || scope == NULL || scope->kind != CONFIGURATION_MODULE_SCOPE) { return entry; }
    return configuration_scope_own(scope->package, key);
}

const ModulePathValue *configuration_alias_lookup(void *context, SourceSpan key) {
    const ConfigurationAliasEntry *entry = configuration_alias_find(context, key);
    return entry == NULL ? NULL : entry->target;
}
