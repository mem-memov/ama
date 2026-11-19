/** @file
 * @brief Own required ordinary-body scheduling independently of Type identity and executable admission.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) retains incoming paths and checks outgoing applications once per closed key.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns partial worklist preparation.
 * [COMP-02](../../../../spec/implementation/compiler.md#comp-02) exposes only complete phase certificates.
 * [TPL-06](../../../../spec/language/templates.md#tpl-06) still requires separate ordinary executable admission.
 */
#ifndef AMA_STATIC_CLOSURE_PLAN_H
#define AMA_STATIC_CLOSURE_PLAN_H
#include "ordinary/pass.h"

/** Keep actual incoming paths even when their complete candidate identity agrees.
 * representative is selected by exact candidate/environment equality, never a
 * source-name heuristic. Its report is inference evidence, not body admission.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) separates shared checking from incoming provenance.
 */
typedef struct StaticClosureEntry {
    struct StaticClosureEntry *next;
    const StaticResultContext *context;
    const StaticTypedScope *scope;
    struct StaticClosureEntry *representative;
    StaticOrdinaryPass report;
} StaticClosureEntry;

/** Preserve the first composed diagnostic; Pending is a successful refinement
 * outcome until the final whole-program acceptance operation.
 * [TPL-10](../../../../spec/language/templates.md#tpl-10) forbids defaulting required unknowns.
 */
typedef enum { STATIC_CLOSURE_OK, STATIC_CLOSURE_INVALID, STATIC_CLOSURE_CONFLICT,
    STATIC_CLOSURE_UNRESOLVED, STATIC_CLOSURE_OUT_OF_MEMORY } StaticClosureStatus;

/** Own only queue records. Connections, all static semantic dependencies and
 * original sources outlive this plan. No runtime instance, native resource or
 * candidate-local executable certificate belongs here.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) requires reverse dependency teardown.
 */
typedef struct {
    SourceArena arena;
    StaticConnectionPlan *connections;
    StaticClosureEntry *first;
    StaticClosureEntry *last;
    size_t count;
    /** Whole-Type certificate from the last successful final resolution.
     * Collection/refinement or a failed phase/dependency invalidates it; retained
     * immutable descriptors are not executable admission.
     * [COMP-02](../../../../spec/implementation/compiler.md#comp-02) requires complete phase outputs before consumption.
     */
    bool closed;
    StaticClosureStatus status;
    Diagnostic error;
} StaticClosurePlan;

/** Borrow healthy connection ownership without allocation or Type expansion.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) separates owners.
 */
void static_closure_init(StaticClosurePlan *, StaticConnectionPlan *, SourceAllocator);
/** Release complete/failed worklist metadata before borrowed owners; zero state.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) bounds teardown.
 */
void static_closure_destroy(StaticClosurePlan *);
/** Discover all registered actual ordinary contexts, retain each incoming path,
 * and choose the first exact-key/environment representative. Bind source scope
 * variables to their canonical owner without merging runtime instances. Static
 * producer contexts remain in the static publication component. On failure keep
 * partial entries owned and leave progress output unchanged. Collection clears
 * the previous whole-Type certificate; call resolve before consuming closure.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) validates actual cached paths before sharing work.
 */
bool static_closure_collect(StaticClosurePlan *, bool *);
/** One whole-worklist refinement observation, separate from ordinary capability
 * checks and final executable admission.
 * [TPL-06](../../../../spec/language/templates.md#tpl-06) requires subsequent ordinary validation.
 */
typedef struct { bool changed; bool types_closed; } StaticClosureStep;
/** Submit one evidence-driven pass through required ordinary bodies, fields,
 * immediate call targets and independently selected constant namespaces. Retain
 * newly discovered contexts for the next pass; do not speculate unknown origins.
 * Output is unchanged on refusal and partial work remains owned.
 * [TPL-10](../../../../spec/language/templates.md#tpl-10) permits exact inference only.
 */
bool static_closure_step(StaticClosurePlan *, StaticClosureStep *);
/** Reach the exact evidence fixed point, then require every retained key and
 * Type obligation to close. On success set closed without completing any
 * candidate-local ordinary/capability certificate. Subsequent input changes need
 * another resolution. Failures preserve first diagnostics and safe teardown.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) closes finite nominal groups without admitting unchecked bodies.
 */
bool static_closure_resolve(StaticClosurePlan *);
#endif
