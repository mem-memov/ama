/** @file
 * @brief Own candidate/use metadata while borrowing guards, terms and collision-checked names.
 * [TPL-05](../../../../spec/language/templates.md#tpl-05) separates shared identities from runtime instances.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns failed preparation.
 * [TPL-08](../../../../spec/language/templates.md#tpl-08) preserves original local identities.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) checks incoming paths before cache use.
 * [NAME-02](../../../../spec/implementation/compiler.md#name-02) preserves full-key collision evidence.
 */
#ifndef AMA_STATIC_REGISTRY_PLAN_H
#define AMA_STATIC_REGISTRY_PLAN_H
#include "model.h"

/** Sticky failures retain original borrowed diagnostics and valid partial records.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03).
 */
typedef enum { STATIC_REGISTRY_OK, STATIC_REGISTRY_INVALID, STATIC_REGISTRY_CONFLICT,
    STATIC_REGISTRY_COLLISION, STATIC_REGISTRY_OUT_OF_MEMORY } StaticRegistryStatus;

/** Destroy before borrowed guards/names and their longer-lived source/term owners.
 * No runtime storage, execution or implicit default instance belongs here.
 * [TPL-05](../../../../spec/language/templates.md#tpl-05) shares compiler metadata only.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) retains partial allocations.
 */
typedef struct {
    SourceArena arena;
    StaticGuardPlan *guards;
    IdentityRegistry *names;
    StaticRegistryCandidate *first_candidate;
    size_t candidate_count;
    StaticRegistryUse *first_use;
    size_t use_count;
    StaticRegistryDependency *first_dependency;
    size_t dependency_count;
    StaticRegistryStatus status;
    Diagnostic error;
} StaticRegistryPlan;

/** Initialize without allocation; borrowed owners must be initialized and outlive
 * this plan. [ARCH-03](../../../../spec/implementation/compiler.md#arch-03).
 */
void static_registry_init(StaticRegistryPlan *, StaticGuardPlan *, IdentityRegistry *, SourceAllocator);
/** Release owned partial metadata without following graph/source references.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03).
 */
void static_registry_destroy(StaticRegistryPlan *);
/** Retain one actual guard-published use and normalize its owner key. Pending
 * succeeds with a stable use handle and no candidate/name. Closed identity is
 * registration only: candidate body/executable checking is not implied.
 * Also retain every actual parent and its validation dependency iteratively.
 * Repeated path queries reuse the handle but still refresh guard constraints.
 * Public receivers change only on success; partial records remain owned.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) validates every incoming use.
 */
bool static_registry_use(StaticRegistryPlan *, const StaticCreationPath *,
    StaticRegistryUse **, StaticRegistryKeyState *);
/** Refresh only known-key associations and guard obligations. This key-closure
 * query is not a body-completion or executable-admission certificate.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) rechecks late constraints.
 */
bool static_registry_refresh_keys(StaticRegistryPlan *, StaticRegistryKeyState *);
/** Equate one original scoped variable with the same source operation in its
 * closed canonical owner. Pending keys add no equation and leave value output
 * untouched; use handles and runtime instances remain distinct. Caller may retry
 * after independent constraints close the owner. Never traverses nominal values.
 * [TPL-05](../../../../spec/language/templates.md#tpl-05) shares specialization metadata.
 * [TPL-08](../../../../spec/language/templates.md#tpl-08) preserves lexical identity.
 */
bool static_registry_scope_variable(StaticRegistryPlan *, StaticRegistryUse *,
    const StaticTerm *, const StaticTerm **, StaticRegistryKeyState *);
/** Retain a validation dependency between registered actual uses. The caller
 * supplies its original source span and separately registers every constructor
 * creation with guards; this operation never manufactures creation provenance.
 * Duplicate endpoint uses reuse the obligation. A later dependency invalidates
 * prior checked state until refresh, without discarding local-check evidence.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) retains cached outgoing obligations.
 */
bool static_registry_depend(StaticRegistryPlan *, StaticRegistryUse *, StaticRegistryUse *, SourceSpan);
/** Claim at most one local-check expansion per closed canonical candidate.
 * Unknown keys return Pending with should_check=false, without an expansion.
 * Once started, a repeat (including another incoming use) returns false in that
 * receiver; it still checks guards and does not imply completed validation.
 * Outputs change only on success. A local phase may also supply an independently
 * completed certificate directly through complete_local.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) closes identity cycles without repeated expansion.
 */
bool static_registry_begin_local(StaticRegistryPlan *, StaticRegistryUse *, bool *, StaticRegistryKeyState *);
/** Record that the caller finished all local checks and enumerated dependencies
 * for this closed candidate. This is a compiler phase certificate, not a body
 * evaluator. It neither proves dependencies nor admits executable code. Calls
 * before the key closes are invalid. Equal-key uses share this local evidence.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) separates registration and checking.
 */
bool static_registry_complete_local(StaticRegistryPlan *, StaticRegistryUse *);
/** Compute the greatest finite checked group: local checks and every outgoing
 * dependency must pass. Same-key cycles may close together; any reachable open
 * key or incomplete local check keeps its predecessors pending. Always refresh
 * actual guard constraints and retain all guard-demanded paths before returning.
 * The fixed-point walk itself allocates no storage and uses no recursion. On failure no closure output is changed.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) checks every closed-cycle branch.
 */
bool static_registry_refresh_checks(StaticRegistryPlan *, StaticRegistryCheckState *);
/** Equate all existing variables in one original owner environment with their
 * canonical equivalents after its key closes. Unknown keys add no equations.
 * New canonical variables are not revisited during this finite scan; later
 * source variables can be included by another call. No role list or runtime
 * instance/value identity is merged. Partial equations remain owned on failure,
 * and the state receiver changes only on success. Body validation is separate.
 * [TPL-05](../../../../spec/language/templates.md#tpl-05) makes one complete-key specialization.
 * [TPL-08](../../../../spec/language/templates.md#tpl-08) preserves original lexical operations.
 */
bool static_registry_scope_environment(StaticRegistryPlan *, StaticRegistryUse *, StaticRegistryKeyState *);
#endif
