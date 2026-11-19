/** @file
 * @brief Evaluate pair definitions and exact alias reads without inventing data dependencies on other outputs.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves structural definitions and immutable first receipts.
 */
#include "internal.h"

/** Selected metadata closure is independent of unrelated global requirements.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12).
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

/** Pair RHS values equate directly. An alias separately enters the exact
 * selected foreign definition under its own path, even when its producer's
 * independent validation roots already computed that value.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) guards real definition cycles.
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
    if (frame->phase == 1) {
        frame->phase = 2;
        if (use->definition->symbol->role == STATIC_RESULT) {
            if (!static_constraint_equate(plan->guards->applications->solver, use->definition->value,
                use->rhs->source->value, use->definition->symbol->site)) {
                (void)static_result_dependencies(plan); return false;
            }
        } else {
            bool pending;
            if (!static_result_alias_read(plan, use, &use->selected, &pending)) { return false; }
            frame->pending |= pending;
            if (!pending) { return static_result_push_definition(evaluation, use->selected); }
        }
    }
    bool closed;
    if (!static_result_closed(plan, use->definition->value, &closed)) { return false; }
    StaticResultOutcome outcome = frame->pending ? STATIC_RESULT_PENDING : STATIC_RESULT_READY;
    static_result_pop(evaluation, outcome, closed && frame->closed);
    bool cached;
    return static_result_cache_store(plan, use, &cached);
}
