/** @file
 * @brief Own structural application snapshots separately from immutable terms and finite solutions.
 * [TPL-07](../../../../spec/language/templates.md#tpl-07) requires exact category/signature checks.
 * [TPL-08](../../../../spec/language/templates.md#tpl-08) preserves captures and scope.
 * [TPL-10](../../../../spec/language/templates.md#tpl-10) fixes unique nominal recovery.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) defines producer inference.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns partial construction.
 */
#ifndef AMA_STATIC_APPLICATION_PLAN_H
#define AMA_STATIC_APPLICATION_PLAN_H
#include "model.h"
#include "../constraints/solver.h"

/** Pending is an operation outcome; sticky failures distinguish invalid caller
 * metadata, semantic disagreement, exhaustion and final underdetermination.
 * [TPL-08](../../../../spec/language/templates.md#tpl-08) rejects invalid applications.
 */
typedef enum { STATIC_APPLY_OK, STATIC_APPLY_INVALID, STATIC_APPLY_CONFLICT,
    STATIC_APPLY_OUT_OF_MEMORY, STATIC_APPLY_UNRESOLVED } StaticApplicationStatus;
/** Resolved certifies present structure/contracts only; independent closure and
 * executable/producer admission remain required.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) requires complete producers.
 */
typedef enum { STATIC_APPLY_RESOLVED, STATIC_APPLY_PENDING } StaticApplicationOutcome;

/** Destroy this owner before its solver, terms, binding plan and sources; never
 * store a runtime instance or implicit default. All failed preparation is owned.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) separates lifetimes.
 */
typedef struct {
    SourceArena arena;
    StaticTermStore *terms;
    StaticConstraintSolver *solver;
    BindingPlan *bindings;
    StaticApplicationRecord *first;
    StaticApplicationRecord *last;
    size_t count;
    StaticApplicationStatus status;
    Diagnostic error;
    DiagnosticRelated related;
} StaticApplicationPlan;

/** Borrow healthy compatible owners without allocation or source mutation.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) separates owners.
 */
void static_application_plan_init(StaticApplicationPlan *, StaticTermStore *,
    StaticConstraintSolver *, BindingPlan *, SourceAllocator);
/** Release all partial snapshots before borrowed owners; zero this owner.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) bounds teardown.
 */
void static_application_plan_destroy(StaticApplicationPlan *);
/** Derive omission from registered lexical syntax, including output-phase
 * override. Publish only on success; no body or namespace member is looked up.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) fixes contextual inference.
 */
bool static_application_context(StaticApplicationPlan *, const StaticTermEnvironment *,
    const AstTypeExpression *, StaticApplicationContext *);
/** Merge an actual constructor's captures with a count-sized original-slot
 * input array. Null values are skips. New bindings cannot reopen captured slots.
 * Pending origins retain a snapshot without inventing a module. Source policies
 * control omission; value/record output is untouched on failure.
 * [TPL-08](../../../../spec/language/templates.md#tpl-08) defines partial application.
 */
bool static_application_forward(StaticApplicationPlan *, const StaticTerm *,
    const StaticApplicationInput *, size_t, StaticApplicationContext, const StaticApplicationRecord **);
/** Re-normalize original requests against the currently known actual residual
 * interface. Preserve RHS identities by original binding syntax, including skips.
 * Unknown origins retain provenance and a lexical result variable for later
 * exact recovery; no expected signature supplies positional slots.
 * [TPL-08](../../../../spec/language/templates.md#tpl-08) preserves structural application.
 */
bool static_application_source(StaticApplicationPlan *, const StaticTermApplication *,
    const StaticApplicationRecord **);
/** Constrain a source application by independently known nominal result facts.
 * Recover its unknown constructor by exact declared residual names, retaining
 * all other captures; unknown origins remain Pending. Never search definitions.
 * [TPL-10](../../../../spec/language/templates.md#tpl-10) fixes unique recovery.
 */
bool static_application_recover(StaticApplicationPlan *, const StaticTermApplication *,
    const StaticTerm *, const StaticApplicationRecord **);
/** Recheck one published snapshot against currently known category/signature
 * evidence. Pending does not select an origin or discharge other admission work.
 * [TPL-07](../../../../spec/language/templates.md#tpl-07) requires exact contracts.
 */
bool static_application_check(StaticApplicationPlan *, const StaticApplicationRecord *, StaticApplicationOutcome *);
#endif
