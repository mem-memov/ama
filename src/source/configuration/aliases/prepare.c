/**
 * @file
 * @brief Allocate just one scope's own alias entries while retaining original key spans.
 * [PKG-04](../../../../spec/project/packages.md#pkg-04) shares immutable package
 * targets rather than copying them per module. [ARCH-03](../../../../spec/implementation/compiler.md#arch-03)
 * owns scratch; [ARCH-05](../../../../spec/implementation/compiler.md#arch-05)
 * checks flexible storage sizing and alignment without a guessed offset.
 */
#include <stdint.h>
#include "internal.h"

/** Compiler-provided flexible-member alignment keeps own entries adjacent to their scope.
 * [ARCH-05](../../../../spec/implementation/compiler.md#arch-05) requires checked
 * offsets and at least the complete struct size, including possible tail padding.
 */
typedef struct {
    ConfigurationAliasScope scope;
    ConfigurationAliasEntry entries[];
} ScopeStorage;

ConfigurationScopeStatus configuration_scope_prepare(SourceArena *arena, const ConfigurationDocument *document,
    bool local, ConfigurationScopeWork *output, SourceIssue *issue) {
    size_t count = 0;
    for (const ConfigurationAlias *alias = document == NULL ? NULL : document->aliases;
        alias != NULL; alias = alias->next) { count++; }
    size_t header = offsetof(ScopeStorage, entries);
    if (count > (SIZE_MAX - header) / sizeof(ConfigurationAliasEntry)) {
        return configuration_scope_failure(issue, (SourceSpan){0}, CONFIGURATION_SCOPE_LIMIT,
            "Alias table size is not representable", "a representable own-alias count");
    }
    size_t size = header + count * sizeof(ConfigurationAliasEntry);
    if (size < sizeof(ScopeStorage)) { size = sizeof(ScopeStorage); }
    ScopeStorage *storage = source_arena_allocate(arena, size);
    if (storage == NULL) {
        return configuration_scope_failure(issue, (SourceSpan){0}, CONFIGURATION_SCOPE_OUT_OF_MEMORY,
            "Alias table allocation failed", "available alias-table storage");
    }
    storage->scope = (ConfigurationAliasScope){.source = document == NULL ? NULL : document->source,
        .entries = count == 0 ? NULL : storage->entries, .count = count};
    size_t index = 0;
    for (const ConfigurationAlias *alias = document == NULL ? NULL : document->aliases;
        alias != NULL; alias = alias->next) {
        storage->entries[index++] = (ConfigurationAliasEntry){.name = alias->name, .local = local};
    }
    *output = (ConfigurationScopeWork){&storage->scope, count == 0 ? NULL : storage->entries};
    return CONFIGURATION_SCOPE_OK;
}
