/**
 * @file
 * @brief Separate alias key preparation, finite scope checks, target reduction and publication.
 * [PKG-04](../../../../spec/project/packages.md#pkg-04) defines exact two-level
 * scope; [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns failed
 * preparation. [ARCH-05](../../../../spec/implementation/compiler.md#arch-05)
 * checks sizes; [COMP-04](../../../../spec/implementation/compiler.md#comp-04)
 * retains original declaration sites.
 */
#ifndef AMA_SOURCE_CONFIGURATION_ALIASES_INTERNAL_H
#define AMA_SOURCE_CONFIGURATION_ALIASES_INTERNAL_H
#include "aliases.h"

/** Mutable preparation view never exposed before all target reductions succeed.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) gives both
 * fields the same arena owner, without transferring partial success.
 */
typedef struct {
    ConfigurationAliasScope *scope;
    ConfigurationAliasEntry *entries;
} ConfigurationScopeWork;

/** Preserve an original issue and classify malformed scope preparation.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) keeps errors
 * attributable without constructing replacement source locations.
 */
ConfigurationScopeStatus configuration_scope_failure(SourceIssue *issue, SourceSpan span,
    ConfigurationScopeStatus status, const char *message, const char *expected);
/** Preserve path issue detail while mapping size/allocation failures to scope outcomes.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) retains the
 * caller arena and refuses any unpublished partial environment.
 */
ConfigurationScopeStatus configuration_scope_path_status(ModulePathStatus status);
/** Allocate one own-table and copy complete original keys without target resolution.
 * [ARCH-05](../../../../spec/implementation/compiler.md#arch-05) checks array
 * arithmetic; [PKG-04](../../../../spec/project/packages.md#pkg-04) retains keys exactly.
 */
ConfigurationScopeStatus configuration_scope_prepare(SourceArena *arena, const ConfigurationDocument *document,
    bool local, ConfigurationScopeWork *output, SourceIssue *issue);
/** Sort whole byte keys and reject duplicates/collisions before normalizing any target.
 * [PKG-04](../../../../spec/project/packages.md#pkg-04) forbids source-order
 * binding and shadowing rather than selecting a preferred declaration.
 */
ConfigurationScopeStatus configuration_scope_keys(ConfigurationScopeWork work,
    const ConfigurationAliasScope *package, SourceIssue *issue);
/** Compare exact byte keys with length tie-breaking, without numeric subtraction overflow.
 * [PKG-04](../../../../spec/project/packages.md#pkg-04) uses complete original names.
 */
int configuration_scope_compare(SourceSpan left, SourceSpan right);
/** Search only one already sorted immutable table.
 * [PKG-04](../../../../spec/project/packages.md#pkg-04) limits lookup to the
 * owning scope and its explicit package parent, never another module.
 */
const ConfigurationAliasEntry *configuration_scope_own(const ConfigurationAliasScope *scope, SourceSpan key);
/** Reduce each new target using only the explicit package provider.
 * [PKG-04](../../../../spec/project/packages.md#pkg-04) prevents module-alias
 * chains by construction; parsed document nodes are not retained after this call.
 */
ConfigurationScopeStatus configuration_scope_targets(SourceArena *arena, ConfigurationScopeWork work,
    const ConfigurationDocument *document, const ConfigurationAliasScope *package, SourceIssue *issue);
#endif
