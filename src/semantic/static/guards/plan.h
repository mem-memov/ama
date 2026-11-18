/** @file
 * @brief Own checked creation paths independently of immutable applications and equation solving.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) guards expansion before following edges.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) retains scoped symbolic owners.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns failed preparation.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves source witnesses.
 */
#ifndef AMA_STATIC_GUARD_PLAN_H
#define AMA_STATIC_GUARD_PLAN_H
#include "model.h"
#include "definition/model.h"

/** Pending is a successful unresolved operation, not a sticky error or default.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) retains missing evidence.
 */
typedef enum { STATIC_GUARD_OK, STATIC_GUARD_INVALID, STATIC_GUARD_CONFLICT,
    STATIC_GUARD_OUT_OF_MEMORY } StaticGuardStatus;

/** Destroy before applications/solver/terms and all borrowed source storage.
 * A rejection retains the complete existing ancestry plus closing use; reporting
 * allocates nothing and later chain rendering can inspect every original frame.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) bounds partial ownership.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) retains original locations.
 */
typedef struct {
    SourceArena arena;
    SourceArena witness_arena;
    StaticApplicationPlan *applications;
    StaticGuardOwner *first_owner;
    size_t owner_count;
    StaticCreationPath *first_path;
    size_t path_count;
    StaticCreationVertex *first_vertex;
    size_t vertex_count;
    size_t edge_count;
    const StaticCreationSearch *creation_witness;
    const StaticCreationEdge *closing_edge;
    const StaticCreationPath *closing_creation_path;
    bool refreshed;
    size_t observed_paths;
    size_t observed_equations;
    size_t observed_metadata;
    size_t observed_requirements;
    StaticGuardClosure closure;
    StaticDefinitionNode *first_definition;
    size_t definition_count;
    size_t definition_edge_count;
    StaticDefinitionPath *first_definition_path;
    size_t definition_path_count;
    size_t observed_definitions;
    size_t observed_definition_edges;
    size_t observed_definition_paths;
    const StaticDefinitionPath *rejected_definition_path;
    const StaticDefinitionPath *repeated_definition_ancestor;
    const StaticDefinitionNode *rejected_definition;
    const StaticDefinitionSearch *definition_witness;
    const StaticDefinitionEdge *definition_closing_edge;
    const StaticCreationPath *rejected_path;
    const StaticCreationPath *repeated_ancestor;
    const StaticModule *rejected_origin;
    SourceSpan rejected_site;
    StaticGuardStatus status;
    Diagnostic error;
    DiagnosticRelated related[2];
} StaticGuardPlan;

/** Borrow a healthy compatible application/solver/value owner without allocation.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) separates lifetimes.
 */
void static_guard_init(StaticGuardPlan *, StaticApplicationPlan *, SourceAllocator);
/** Release only guard-owned records, then zero the owner; no source traversal.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) bounds teardown.
 */
void static_guard_destroy(StaticGuardPlan *);
/** Intern an exact lexical owner, preparing its complete nominal shape from
 * original scoped formal identities if its environment is generic. Variables
 * remain variables; no extra parameter, constructor default or origin is chosen.
 * Output is unchanged on refusal; any partial allocations remain owned.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) preserves symbolic keys.
 */
bool static_guard_owner(StaticGuardPlan *, const StaticTermEnvironment *, const StaticGuardOwner **);
/** Register an explicit selected compilation root with original definition
 * provenance. A generic environment keeps its source formals symbolic; later
 * admission still requires a unique concrete root. Repeated roots reuse identity.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) requires root provenance.
 */
bool static_creation_root(StaticGuardPlan *, const StaticTermEnvironment *, const StaticCreationPath **);
/** Check a source-backed application of this exact lexical owner before any
 * definition expansion. Finite argument expressions use their common lexical
 * parent. Only ENTERED publishes a path; other successful outcomes leave it
 * untouched. Known origins are checked even when captures remain unresolved.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) checks creation ancestry.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) requires producer phases.
 */
bool static_creation_enter(StaticGuardPlan *, const StaticCreationPath *,
    const StaticApplicationRecord *, const StaticCreationPath **, StaticCreationOutcome *);
/** Normalize registered creation/definition keys and retain all original edges.
 * Check cached descendants and definition cycles after complete-key merging.
 * Pending keys remain scoped records. Closure concerns these registered owners
 * only; executable admission and unknown application requests remain separate.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) rechecks merged incoming paths.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) rechecks scoped definition cycles.
 */
bool static_guard_refresh(StaticGuardPlan *, StaticGuardClosure *);
/** Enter a required result/receiver-alias definition in its checked producer
 * environment. Parameter names and runtime fields are not definitions here.
 * Parent is null for a root; otherwise site must belong to the parent's original
 * source. Roots use the original symbol introduction as their site. Record that
 * dependency and check active/cached definition paths before
 * exposing the target. Creation ancestry remains a distinct checked path.
 * Output is unchanged on failure; retained partial metadata stays owned.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) rejects definition cycles.
 */
bool static_definition_enter(StaticGuardPlan *, const StaticDefinitionPath *,
    const StaticCreationPath *, const StaticSymbol *, SourceSpan, const StaticDefinitionPath **);
#endif
