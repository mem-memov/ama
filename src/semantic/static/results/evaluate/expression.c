/** @file
 * @brief Advance lexical values and named producer selections with an explicit evaluation stack.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) reads original lexical symbols, not captured variable roles.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) guards actual creation before following outputs.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) separates selected reads and whole-graph validation.
 * [TPL-15](../../../../../spec/language/templates.md#tpl-15) delays declaration
 * scalar equations until the selected definition and all supplier inputs close.
 */
#include "internal.h"

/** A derived local source name reads its definition in this actual context.
 * Supplied parameter metadata is already a value, even if its underlying term
 * came from another scope's derived variable; do not search unrelated contexts.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) preserves lexical substitution.
 */
static bool head(StaticResultEvaluation *evaluation, StaticResultFrame *frame) {
    StaticResultUse *use = frame->expression;
    const StaticSymbol *symbol = use->source->occurrence->symbol;
    frame->phase = 2;
    if (!symbol || (symbol->role != STATIC_RESULT && symbol->role != STATIC_ALIAS)) { return true; }
    StaticResultDefinitionUse *definition;
    if (!static_result_definition_use(evaluation->plan, use->context, symbol,
        use->definition_path, use->source->expression->head, &definition)) { return false; }
    return static_result_push_definition(evaluation, definition);
}

/** Every step advances a phase or pushes an original finite dependency. No
 * whole-graph validation wait is placed on the value stack; its independent
 * roots remain required through use->required_graph.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) keeps equality cycles separate.
 */
bool static_result_expression_step(StaticResultEvaluation *evaluation) {
    StaticResultFrame *frame = evaluation->top;
    StaticResultUse *use = frame->expression;
    StaticResultPlan *plan = evaluation->plan;
    if (frame->phase == 0) {
        if (!static_result_prepare(plan, use)) { return false; }
        frame->input = use->requests->first;
        frame->phase = 1;
        return true;
    }
    if (frame->phase == 1) { return head(evaluation, frame); }
    if (frame->phase == 2) {
        while (frame->input) {
            const BindingRequest *input = frame->input;
            frame->input = input->next;
            if (input->direction != BINDING_STATIC_INPUT) { continue; }
            StaticResultUse *child;
            if (!static_result_use(plan, use->context, input->syntax.static_binding->input,
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
    if (frame->phase == 4) {
        frame->phase = 5;
        if (!static_result_selection_read(evaluation, use)) { return false; }
        return true;
    }
    if (!static_result_receivers(plan, use) || !static_result_selection_equation(plan, use)) { return false; }
    bool closed;
    if (use->source->producer) {
        if (!static_result_producer_closed(plan, use, &closed)) { return false; }
    } else if (!static_result_closed(plan, use->source->value, &closed)) { return false; }
    static_result_pop(evaluation, frame->pending ? STATIC_RESULT_PENDING : STATIC_RESULT_READY,
        closed && frame->closed);
    return true;
}
