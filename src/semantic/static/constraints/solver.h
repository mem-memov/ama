/** @file
 * @brief Own incremental finite equations separately from borrowed canonical term storage.
 * [TPL-10](../../../../spec/language/templates.md#tpl-10) keeps solutions outside source definitions.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns failed preparation.
 */
#ifndef AMA_STATIC_CONSTRAINT_SOLVER_H
#define AMA_STATIC_CONSTRAINT_SOLVER_H
#include "model.h"

/** Distinguish malformed inputs/resource refusal from mathematical conflicts and
 * required underdetermination. [TPL-10](../../../../spec/language/templates.md#tpl-10).
 */
typedef enum { STATIC_CONSTRAINT_OK, STATIC_CONSTRAINT_INVALID,
    STATIC_CONSTRAINT_OUT_OF_MEMORY, STATIC_CONSTRAINT_CONFLICT,
    STATIC_CONSTRAINT_UNRESOLVED } StaticConstraintStatus;

/** Complete means the present equations and explicit closure requirements are
 * satisfied; later evidence may still conflict. Pending is not a failure/default.
 * [TPL-10](../../../../spec/language/templates.md#tpl-10) retains delayed evidence.
 */
typedef enum { STATIC_CONSTRAINT_COMPLETE, STATIC_CONSTRAINT_PENDING } StaticConstraintOutcome;

/** Healthy pending owners may accept further finite evidence; a failed owner is
 * only destructible. Term/source owners outlive this arena and diagnostics.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) defines ownership.
 */
typedef struct {
    SourceArena arena;
    StaticTermStore *terms;
    StaticConstraintEquation *first_equation;
    StaticConstraintEquation *last_equation;
    size_t equation_count;
    StaticConstraintVariable *first_variable;
    size_t variable_count;
    StaticConstraintVariable **variables;
    size_t variable_capacity;
    StaticConstraintRequirement *first_requirement;
    StaticConstraintRequirement *last_requirement;
    size_t requirement_count;
    StaticConstraintMetadata *first_metadata;
    size_t metadata_count;
    StaticConstraintSignature *first_signature;
    size_t signature_count;
    const StaticTerm *pending_left;
    const StaticTerm *pending_right;
    SourceSpan pending_site;
    bool dirty;
    StaticConstraintOutcome outcome;
    StaticConstraintStatus status;
    Diagnostic error;
    DiagnosticRelated related[2];
} StaticConstraintSolver;

/** Initialize without allocation; borrow a healthy canonical term owner.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) separates owners.
 */
void static_constraint_init(StaticConstraintSolver *, StaticTermStore *, SourceAllocator);
/** Free all partial solver scratch without following source/value/parent edges.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) bounds teardown.
 */
void static_constraint_destroy(StaticConstraintSolver *);
/** Add an owned term equality, deduplicating symmetric pairs without losing any
 * mathematical constraint. Site is absent or an original valid application span.
 * This prepares evidence; it does not execute a solve or claim consistency.
 * [TPL-10](../../../../spec/language/templates.md#tpl-10) collects finite equations.
 */
bool static_constraint_equate(StaticConstraintSolver *, const StaticTerm *, const StaticTerm *, SourceSpan);
/** Require a specific value to close before final acceptance; duplicate requests
 * reuse the existing obligation. No default/anchor is manufactured.
 * [TPL-10](../../../../spec/language/templates.md#tpl-10) requires unique solutions.
 */
bool static_constraint_require(StaticConstraintSolver *, const StaticTerm *, SourceSpan);
/** Reach an exact finite equality fixed point without expanding definitions.
 * Pending owners may receive further equations. Failure leaves output untouched
 * and preserves source/term payloads; destroy the failed solver before borrowers.
 * [TPL-10](../../../../spec/language/templates.md#tpl-10) requires exact refinement.
 */
bool static_constraint_solve(StaticConstraintSolver *, StaticConstraintOutcome *);
/** Supply independent fully resolved category/signature evidence for one exact
 * term. Preserve all known source restrictions; repeated facts preserve existing
 * publications and conflicting facts fail. Signature storage outlives this solver.
 * Type requires null signature; Template requires a nonempty finite acyclic,
 * distinct-name signature with no deferred entries. No source body is executed.
 * [TPL-10](../../../../spec/language/templates.md#tpl-10) permits only exact refinement.
 */
bool static_constraint_describe(StaticConstraintSolver *, const StaticTerm *, StaticCategory,
    const StaticSignature *, SourceSpan);
/** Query equality entailed by present equations, including unresolved symbolic
 * terms. True is a proof under every completion; false means not yet established,
 * not disequality. Solve existing facts, compare full finite structure, and add
 * no equations/requirements or definition expansions. Query scratch is released
 * on every outcome; failure leaves output untouched. This does not close a
 * specialization key or replace TPL-11's concrete creation-path comparison.
 * [TPL-10](../../../../spec/language/templates.md#tpl-10) preserves exact structural inference.
 */
bool static_constraint_equivalent(StaticConstraintSolver *, const StaticTerm *, const StaticTerm *, bool *);
/** Read a proven nominal outer constructor after solving current equations,
 * without requiring its captures to close or substituting their children.
 * Pending means no origin is known; the nominal output is then unchanged.
 * Complete returns an immutable borrowed nominal term, possibly symbolic or
 * residual, not a concrete specialization/admission certificate. Existing
 * requirements may remain Pending. Failure leaves both outputs unchanged.
 * [TPL-10](../../../../spec/language/templates.md#tpl-10) permits interface facts only from a proven origin.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) separately guards actual definition expansion.
 */
bool static_constraint_nominal(StaticConstraintSolver *, const StaticTerm *,
    const StaticTerm **, StaticConstraintOutcome *);
/** Finalize the present evidence: a stable Pending result becomes a precise
 * underdetermination error. Call incremental solve while further evidence remains.
 * [TPL-10](../../../../spec/language/templates.md#tpl-10) rejects required unknowns.
 */
bool static_constraint_finish(StaticConstraintSolver *);
/** Check consistency and obtain this value's canonical fully closed substitution.
 * A pending selected value leaves value output unchanged and reports Pending;
 * this does not admit other required values or publish producer outputs. May
 * append canonical terms and derived metadata, never modify prior term payloads.
 * [TPL-10](../../../../spec/language/templates.md#tpl-10) separates solving from admission.
 */
bool static_constraint_value(StaticConstraintSolver *, const StaticTerm *,
    const StaticTerm **, StaticConstraintOutcome *);
#endif
