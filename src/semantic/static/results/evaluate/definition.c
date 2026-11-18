/** @file
 * @brief Evaluate original static definitions while keeping aliases distinct from their producer Types.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) defines immutable selected results.
 */
#include "internal.h"

/** Selected queries do not use unrelated root closure as a cache predicate.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) closes actual producers.
 */
bool static_result_closed(StaticResultPlan *plan, const StaticTerm *term, bool *closed) {
    const StaticTerm *value;
    StaticConstraintOutcome outcome;
    if (!static_constraint_value(plan->guards->applications->solver, term, &value, &outcome)) {
        (void)static_result_dependencies(plan); return false;
    }
    *closed = outcome == STATIC_CONSTRAINT_COMPLETE;
    return true;
}

/** Explicit outputs equate their own RHS; aliases receive their value only
 * through their exact original receiver equation in the producer expression.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves denotation.
 */
bool static_result_definition_step(StaticResultEvaluation *evaluation) {
    StaticResultFrame *frame = evaluation->top;
    StaticResultDefinitionUse *use = frame->definition;
    StaticResultPlan *plan = evaluation->plan;
    if (frame->phase == 0) {
        if (!static_result_use(plan, use->context, use->definition->source->expression, use->path, &use->rhs)) { return false; }
        frame->phase = 1;
        return static_result_push_expression(evaluation, use->rhs);
    }
    if (use->definition->symbol->role == STATIC_RESULT &&
        !static_constraint_equate(plan->guards->applications->solver, use->definition->value,
            use->rhs->source->value, use->definition->symbol->site)) {
        (void)static_result_dependencies(plan); return false;
    }
    bool closed;
    if (!static_result_closed(plan, use->definition->value, &closed)) { return false; }
    StaticResultOutcome outcome = frame->pending ? STATIC_RESULT_PENDING : STATIC_RESULT_READY;
    static_result_pop(evaluation, outcome, closed && frame->closed);
    bool cached;
    return static_result_cache_store(plan, use, &cached);
}
