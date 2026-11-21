/**
 * @file
 * @brief Reduce new scope targets from original definitions using only the package provider.
 * [PKG-04](../../../../spec/project/packages.md#pkg-04) prevents local/self chains
 * by construction. Document nodes are borrowed only while constructing results;
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns copied target bytes.
 */
#include "internal.h"

/** Recover one immutable declaration by its complete key without retaining document-arena pointers.
 * [PKG-04](../../../../spec/project/packages.md#pkg-04) gives exact unique names;
 * the document's recorded source order cannot change the selected target.
 */
static const ConfigurationAlias *definition(const ConfigurationDocument *document, SourceSpan key) {
    for (const ConfigurationAlias *alias = document->aliases; alias != NULL; alias = alias->next) {
        if (configuration_scope_compare(alias->name, key) == 0) { return alias; }
    }
    return NULL;
}

ConfigurationScopeStatus configuration_scope_targets(SourceArena *arena, ConfigurationScopeWork work,
    const ConfigurationDocument *document, const ConfigurationAliasScope *package, SourceIssue *issue) {
    for (size_t index = 0; index < work.scope->count; index++) {
        const ConfigurationAlias *alias = definition(document, work.entries[index].name);
        if (alias == NULL) {
            return configuration_scope_failure(issue, work.entries[index].name, CONFIGURATION_SCOPE_INVALID,
                "Prepared alias has no original definition", "an unchanged valid parsed alias document");
        }
        const ModulePathValue *target = NULL;
        ModulePathStatus status = module_path_normalize(arena, alias->target, work.scope->base,
            configuration_alias_lookup, (void *)package, &target, issue);
        if (status != MODULE_PATH_OK) { return configuration_scope_path_status(status); }
        work.entries[index].target = target;
    }
    return CONFIGURATION_SCOPE_OK;
}
