/** @file
 * @brief Advance lexical expression dependencies through an explicit continuation stack.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) preserves nested lexical scope.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) guards required creation paths.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) validates all requested producer outputs.
 */
#include "internal.h"
#include "../../internal.h"

/** Read a derived head in every registered actual context of its original
 * environment. No matching module name can substitute for missing provenance.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves definition scope.
 */
static bool head(StaticResultEvaluation *evaluation, StaticResultFrame *frame) {
    StaticResultUse *use = frame->expression;
    const StaticTerm *term = use->source->head;
    if (term->kind != STATIC_TERM_VARIABLE || term->as.variable.role != STATIC_VARIABLE_DERIVED) {
        frame->phase = 2; return true;
    }
    while (frame->context) {
        const StaticResultContext *context = frame->context;
        frame->context = context->next;
        if (context->creation->owner->environment != term->as.variable.environment) { continue; }
        frame->found_context = true;
        StaticResultDefinitionUse *definition;
        if (!static_result_definition_use(evaluation->plan, context, term->as.variable.symbol,
            use->definition_path, use->source->expression->head, &definition)) { return false; }
        return static_result_push_definition(evaluation, definition);
    }
    frame->pending |= !frame->found_context;
    frame->phase = 2;
    return true;
}

/** Traverse every declared output after explicit output syntax, even a skip.
 * Actual producer environments determine result identities and source order.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) checks whole producer interfaces.
 */
static bool outputs(StaticResultEvaluation *evaluation, StaticResultFrame *frame) {
    StaticResultUse *use = frame->expression;
    const BindingApplication *requests = use->application->context.requests;
    if (requests != NULL && requests->output_count != 0) {
        if (use->producer == NULL) { frame->pending = true; }
        else {
            const StaticModule *module = use->producer->creation->owner->environment->definition;
            if (frame->output < module->result_count) {
                const StaticSymbol *symbol = static_symbol_at(module, STATIC_RESULT, frame->output++);
                StaticResultDefinitionUse *definition;
                if (!static_result_definition_use(evaluation->plan, use->producer, symbol,
                    use->definition_path, use->source->expression->span, &definition)) { return false; }
                return static_result_push_definition(evaluation, definition);
            }
        }
    }
    frame->phase = 5;
    return true;
}

/** Each call advances a phase or pushes one child. A finite source operator has
 * finite inputs/outputs; guarded definitions cannot recur on an active path.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) bounds structural work.
 */
bool static_result_expression_step(StaticResultEvaluation *evaluation) {
    StaticResultFrame *frame = evaluation->top;
    StaticResultUse *use = frame->expression;
    StaticResultPlan *plan = evaluation->plan;
    if (frame->phase == 0) {
        if (!static_result_prepare(plan, use)) { return false; }
        frame->input = use->prepared->first;
        frame->context = plan->first_context;
        frame->phase = 1;
        return true;
    }
    if (frame->phase == 1) { return head(evaluation, frame); }
    if (frame->phase == 2) {
        while (frame->input) {
            const StaticTermInput *input = frame->input;
            frame->input = input->next;
            if (input->value == NULL) { continue; }
            StaticResultUse *child;
            if (!static_result_use(plan, use->context, input->request->syntax.static_binding->input,
                use->definition_path, &child)) { return false; }
            return static_result_push_expression(evaluation, child);
        }
        frame->phase = 3;
        return true;
    }
    if (frame->phase == 3) {
        StaticApplicationOutcome outcome;
        if (!static_result_apply(plan, use, &outcome)) { return false; }
        frame->pending |= outcome == STATIC_APPLY_PENDING;
        frame->phase = 4;
        return true;
    }
    if (frame->phase == 4) { return outputs(evaluation, frame); }
    if (!static_result_receivers(plan, use)) { return false; }
    bool closed;
    if (!static_result_closed(plan, use->source->value, &closed)) { return false; }
    static_result_pop(evaluation, frame->pending ? STATIC_RESULT_PENDING : STATIC_RESULT_READY,
        closed && frame->closed);
    return true;
}
