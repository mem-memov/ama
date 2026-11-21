/** @file
 * @brief Share exact membership, first-error and partial-allocation guard operations.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns partial records.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) retains borrowed diagnostics.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) preserves checked path identities.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) separates definition dependencies.
 */
#ifndef AMA_STATIC_GUARD_INTERNAL_H
#define AMA_STATIC_GUARD_INTERNAL_H
#include "plan.h"
#include "../applications/internal.h"
#include "../terms/internal.h"

/** Allocate zeroed owned records while healthy; errors require no allocation.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns partial preparation.
 */
void *static_guard_allocate(StaticGuardPlan *, size_t);
/** Preserve the first constant diagnostic and existing source witness fields.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) avoids reporting allocation.
 */
bool static_guard_fail(StaticGuardPlan *, StaticGuardStatus, SourceSpan, const char *);
/** Propagate compatible borrowed-owner errors without copying their storage.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves original diagnostics.
 */
bool static_guard_dependencies(StaticGuardPlan *);
/** Validate an exact published path before following its ancestry or source.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) preserves incoming uses.
 */
bool static_guard_has_path(const StaticGuardPlan *, const StaticCreationPath *);
/** Compare a borrowed candidate owner without publishing its symbolic scope.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) suspends unknown repeats.
 */
bool static_guard_owner_ancestry(StaticGuardPlan *, const StaticCreationPath *,
    const StaticGuardOwner *, SourceSpan, bool *);
/** Suspend an unresolved source-only ancestor's repeated origin reachable through
 * cached incoming edges, even when absent from its physical parent chain.
 * Actual paths keep their own ancestry; closed metadata owners retain finite
 * canonical scope identity and must not inherit unrelated pending callers.
 * Pending publishes no new target environment or path. Complete unequal paths are
 * still rejected by the ordinary entry/graph checks before scope expansion.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) preserves cache ancestry.
 */
bool static_guard_cached_pending(StaticGuardPlan *, const StaticCreationPath *,
    const StaticGuardOwner *, bool *);
/** Compare repeated actual template origins by complete keys before expansion.
 * Pending keys suspend this request, without guessing equality or another scope.
 * Closed unequal keys retain both canonical identities and the original path.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) permits only exact-key closure.
 */
bool static_guard_ancestry(StaticGuardPlan *, const StaticCreationPath *, const StaticTerm *, SourceSpan, bool *);
/** Append one prechecked immutable occurrence; counters fail before publication.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) retains every creation edge.
 */
bool static_guard_path(StaticGuardPlan *, const StaticCreationPath *, const StaticGuardOwner *,
    const StaticApplicationRecord *, SourceSpan, bool, const StaticCreationPath **);
/** Intern one already normalized complete owner environment as a graph key.
 * Caller proves normalization/closure through T176; this helper chooses no key.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) preserves concrete identities.
 */
bool static_guard_vertex(StaticGuardPlan *, const StaticTermEnvironment *, StaticCreationVertex **);
/** Normalize every owner input using the solver before sharing a concrete key.
 * Output is unchanged while pending. Producer environments never become Types.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) preserves producer keys.
 */
bool static_guard_owner_key(StaticGuardPlan *, const StaticGuardOwner *,
    const StaticTermEnvironment **, StaticConstraintOutcome *);
/** Add every newly concrete path edge once without dropping original uses.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) preserves incoming provenance.
 */
bool static_guard_edges(StaticGuardPlan *);
/** Check reachable distinct complete keys of the same template origin with
 * finite iterative reachability through concrete and unresolved scope classes,
 * preserving both endpoint keys and a full cached-edge witness on failure.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) rechecks cached descendants.
 */
bool static_guard_creation_graph(StaticGuardPlan *);
/** Normalize symbolic creation owners while retaining pending frames and all
 * exact original environments.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) preserves complete keys.
 */
bool static_guard_creation_keys(StaticGuardPlan *, StaticGuardClosure *);
/** Intern one original result/alias in its exact required producer scope.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) preserves definition identity.
 */
bool static_guard_definition(StaticGuardPlan *, const StaticCreationPath *, const StaticSymbol *, StaticDefinitionNode **);
/** Normalize definition owners and merge only equal complete owner/symbol keys.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) preserves all incoming edges.
 */
bool static_guard_definition_keys(StaticGuardPlan *, StaticGuardClosure *);
/** Check the separate scoped definition relation for directed cycles.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) forbids definition recursion.
 */
bool static_guard_definition_graph(StaticGuardPlan *);
/** Deduplicate only an exact original source/target/use dependency occurrence.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) retains original edges.
 */
bool static_guard_definition_edge(StaticGuardPlan *, StaticDefinitionNode *, StaticDefinitionNode *,
    SourceSpan, const StaticDefinitionEdge **);
/** Retain each distinct incoming definition/creation ancestry before expansion.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) preserves active paths.
 */
bool static_guard_definition_path(StaticGuardPlan *, const StaticDefinitionPath *, const StaticCreationPath *,
    StaticDefinitionNode *, const StaticDefinitionEdge *, SourceSpan, const StaticDefinitionPath **);
#endif
