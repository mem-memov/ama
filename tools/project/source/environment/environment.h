/** @file
 * @brief Own exact package alias/selection environments over one actual local catalog.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * requires original/final scopes and finite selection domains, not borrowed
 * caller permissions or guessed mask rewrites.
 */
#ifndef AMA_SOURCE_PACKAGE_ENVIRONMENT_H
#define AMA_SOURCE_PACKAGE_ENVIRONMENT_H
#include "source/configuration/selection/catalog.h"
/** Preparation failures never publish valid package environments. */
typedef enum { SOURCE_ENVIRONMENT_OK, SOURCE_ENVIRONMENT_INVALID,
    SOURCE_ENVIRONMENT_MEMORY, SOURCE_ENVIRONMENT_LIMIT,
    SOURCE_ENVIRONMENT_CONFIGURATION } SourceEnvironmentStatus;
/** Preserve the failing neutral operation for diagnostics without string inference. */
typedef enum { SOURCE_ENVIRONMENT_VIEW, SOURCE_ENVIRONMENT_DOCUMENT,
    SOURCE_ENVIRONMENT_ALIASES, SOURCE_ENVIRONMENT_SELECTORS,
    SOURCE_ENVIRONMENT_SELECTION } SourceEnvironmentPhase;
/** Own one document arena; scope/selection borrow the enclosing owner arena.
 * Version/configuration source bytes borrow the immutable catalog through teardown.
 */
typedef struct SourcePackageScope {
    struct SourcePackageScope *next;
    const ModuleVersion *version;
    ConfigurationDocument document;
    const ConfigurationAliasScope *aliases;
    const DependencySelection *selection;
} SourcePackageScope;
/** Single-attempt owner. Failed initialized document rows remain destructible;
 * valid certifies all actual ordinary version scopes, not a copy permission plan.
 * No standard package environment is fabricated. Do not copy a live owner.
 */
typedef struct {
    SourceArena arena;
    const ModuleCatalog *catalog;
    const DependencyView *view;
    SourcePackageScope *first, *last;
    const ModuleVersion *failed_version;
    SourceIssue issue;
    SourceEnvironmentStatus status;
    SourceEnvironmentPhase phase;
    bool attempted, valid;
} SourcePackageEnvironment;
/** Begin one owner with no discovery, parsing or source effects. */
void source_package_environment_init(SourcePackageEnvironment *, SourceAllocator);
/** Destroy every initialized document before shared metadata/catalog expiration. */
void source_package_environment_destroy(SourcePackageEnvironment *);
/** Build once from one successfully built immutable catalog. Every retained
 * version's configuration is parsed in its own exact version scope. Use the
 * shared actual universe, alias normalizer, selector compiler and union evaluator.
 * No I/O/compiler state/source mutation; no implicit network or fallback.
 * Output becomes valid only after all scopes succeed. Retain partial ownership,
 * failure phase/version and original issue on refusal. Repeated calls refuse
 * unchanged. Finite version/selector/domain scans and shared exact algorithms
 * bound preparation; arena owns all scratch and document arenas own parsed rows.
 * This supplies facts only: mapping, authorized additions, final set/edge laws,
 * freshness and transaction acceptance remain mandatory outer gates.
 */
SourceEnvironmentStatus source_package_environment_build(SourcePackageEnvironment *, const ModuleCatalog *);
#endif
