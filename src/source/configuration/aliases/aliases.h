/**
 * @file
 * @brief Prepare immutable package/local alias scopes without importing a module.
 * [PKG-04](../../../../spec/project/packages.md#pkg-04) fixes distinct bases,
 * nonshadowing and bounded expansion; [SRC-06](../../../../spec/language/source-and-names.md#src-06)
 * supplies path reduction. [PKG-01](../../../../spec/project/packages.md#pkg-01)
 * identifies the owning version. [ARCH-03](../../../../spec/implementation/compiler.md#arch-03)
 * retains partial arena preparation; [ARCH-05](../../../../spec/implementation/compiler.md#arch-05)
 * checks storage sizes. [COMP-04](../../../../spec/implementation/compiler.md#comp-04)
 * preserves original refusal sites.
 */
#ifndef AMA_SOURCE_CONFIGURATION_ALIASES_H
#define AMA_SOURCE_CONFIGURATION_ALIASES_H
#include "source/configuration/document/document.h"
#include "source/path/normalize/normalize.h"

/** Source/schema/scope refusal stays distinct from size/allocation limits.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) never publishes
 * a partial scope as a successful configuration environment.
 */
typedef enum {
    CONFIGURATION_SCOPE_OK,
    CONFIGURATION_SCOPE_INVALID,
    CONFIGURATION_SCOPE_LIMIT,
    CONFIGURATION_SCOPE_OUT_OF_MEMORY
} ConfigurationScopeStatus;

/** Only a package scope may be the parent of a module scope.
 * [PKG-04](../../../../spec/project/packages.md#pkg-04) forbids inheriting a
 * parent module's local aliases or building a module-alias chain.
 */
typedef enum { CONFIGURATION_PACKAGE_SCOPE, CONFIGURATION_MODULE_SCOPE } ConfigurationScopeKind;

/** Whole original alias key and already reduced target; source views remain borrowed.
 * local distinguishes module declarations from shared package entries without
 * guessing a physical filename. [PKG-04](../../../../spec/project/packages.md#pkg-04)
 * preserves exact case/Unicode/name spelling and introducing scope.
 */
typedef struct {
    SourceSpan name;
    const ModulePathValue *target;
    bool local;
} ConfigurationAliasEntry;

/** Immutable arena-owned own-table plus at most one shared package parent.
 * Entries are sorted by exact UTF-8 bytes. A module retains its parent's immutable
 * targets rather than copying the complete shared table for every module.
 * The parent and original SourceText owners outlive the module scope. Parsed
 * document arenas need survive construction only. [PKG-04](../../../../spec/project/packages.md#pkg-04)
 * fixes this two-level lookup domain; [ARCH-03](../../../../spec/implementation/compiler.md#arch-03)
 * makes all borrowing explicit.
 */
typedef struct ConfigurationAliasScope {
    ConfigurationScopeKind kind;
    const ModulePathValue *base;
    const ModulePathValue *module;
    const struct ConfigurationAliasScope *package;
    const SourceText *source;
    const ConfigurationAliasEntry *entries;
    size_t count;
} ConfigurationAliasScope;

/** Build package aliases against one exact canonical version identity.
 * Document is a valid immutable package-profile result. No target is loaded;
 * all targets are direct raw paths. Output changes only after every key/target
 * succeeds; arena scratch remains owned on failure. [PKG-04](../../../../spec/project/packages.md#pkg-04)
 * uses the version directory rather than a consumer module as the relative base.
 */
ConfigurationScopeStatus configuration_package_aliases(SourceArena *arena,
    const ConfigurationDocument *document, const unsigned char *version, size_t length,
    const ConfigurationAliasScope **output, SourceIssue *issue);

/** Build local aliases for one exact complete module identity.
 * A valid matching package scope is mandatory for ordinary modules and forbidden
 * for standard modules. Optional NULL document means no companion, never an
 * invented alias file. A supplied document is a valid module-profile result.
 * Check all local/shared collisions before resolving targets; local right sides
 * can use only the package table. [PKG-04](../../../../spec/project/packages.md#pkg-04)
 * forbids shadowing/chaining; [PKG-01](../../../../spec/project/packages.md#pkg-01)
 * prevents a foreign version's scope from leaking into this module.
 */
ConfigurationScopeStatus configuration_module_aliases(SourceArena *arena,
    const ConfigurationDocument *document, const ConfigurationAliasScope *package,
    const unsigned char *module, size_t length, const ConfigurationAliasScope **output,
    SourceIssue *issue);

/** Return the complete visible entry, or NULL for absent/invalid key/scope.
 * No allocation or recursion occurs: own table then at most one package table.
 * Returned entry/data remain borrowed from their declared scope owners.
 * [PKG-04](../../../../spec/project/packages.md#pkg-04) forbids shorter-name
 * fallback, normalization or implicit lookup of another module's local table.
 */
const ConfigurationAliasEntry *configuration_alias_find(const ConfigurationAliasScope *scope, SourceSpan key);

/** Adapt immutable scope lookup to the pure normalizer's synchronous target callback.
 * No source span or caller resource is retained. [PKG-04](../../../../spec/project/packages.md#pkg-04)
 * expands only the complete leading key; no target contains deferred alias text.
 */
const ModulePathValue *configuration_alias_lookup(void *context, SourceSpan key);
#endif
