/** @file
 * @brief Require exact whole-worklist Type closure after evidence-driven refinement stabilizes.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) rejects required underdetermination without defaulting.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) keeps executable admission separate from closed identities.
 */
#include "../internal.h"

/** Each repeating pass adds an exact constraint, required context/path/key or
 * first proof receipt. Creation guards and canonical representative sharing
 * bound expansion; no retry/time/depth limit substitutes for that invariant.
 * Candidate-local checks remain unclaimed for the following semantic phase.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) permits finite same-key cycles.
 */
bool static_closure_resolve(StaticClosurePlan *plan) {
    if (!static_closure_dependencies(plan)) { return false; }
    StaticClosureStep report;
    do {
        if (!static_closure_step(plan, &report)) { return false; }
    } while (report.changed);
    StaticConstraintSolver *solver = plan->connections->constants->publications->results->guards->applications->solver;
    if (!static_constraint_finish(solver)) {
        if (solver->status == STATIC_CONSTRAINT_UNRESOLVED) {
            plan->status = STATIC_CLOSURE_UNRESOLVED;
            plan->error = solver->error;
        } else { (void)static_closure_dependencies(plan); }
        return false;
    }
    if (!report.types_closed) {
        SourceSpan site = plan->first ? plan->first->context->creation->site : (SourceSpan){0};
        return static_closure_fail(plan, STATIC_CLOSURE_UNRESOLVED, site,
            "Required specialization keys, field Types or operation dependencies remain unresolved");
    }
    plan->closed = true;
    return true;
}
