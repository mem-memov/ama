/** @file
 * @brief Prepare one source operator with lexical child values and explicit named producer maps.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) substitutes scoped structure.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) separates input and output actions.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns partial preparation.
 */
#include "../internal.h"

/** Publicly retained work identity must be checked before its payload is read.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves original uses.
 */
bool static_result_has_use(const StaticResultPlan *plan, const StaticResultUse *candidate) {
    for (const StaticResultUse *use = plan->first_use; use; use = use->next) {
        if (use == candidate) { return true; }
    }
    return false;
}

/** Share source-only operand preparation, then retain every actual lexical child
 * use with this incoming definition path. Source metadata has no readiness proof;
 * evaluation still visits each child through the actual creation context.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) preserves lexical scope.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves actual dependency ancestry.
 */
bool static_result_prepare(StaticResultPlan *plan, StaticResultUse *use) {
    if (!static_result_dependencies(plan)) { return false; }
    if (!static_result_has_use(plan, use)) {
        return static_result_fail(plan, STATIC_RESULT_INVALID, (SourceSpan){0}, "Invalid expression preparation handle");
    }
    if (use->requests != NULL) { return true; }
    const StaticTermApplication *prepared = NULL;
    const StaticResultProducer *producer = NULL;
    const BindingApplication *requests;
    if (use->source->producer) {
        if (!static_result_prepare_producer(plan, use->source, &producer)) { return false; }
        requests = producer->requests;
    } else {
        if (!static_result_operands(plan, use->source, &prepared)) { return false; }
        requests = prepared->source;
    }
    for (const BindingRequest *input = requests->first; input; input = input->next) {
        if (input->direction != BINDING_STATIC_INPUT) { continue; }
        StaticResultUse *child;
        if (!static_result_use(plan, use->context, input->syntax.static_binding->input,
            use->definition_path, &child)) { return false; }
    }
    use->requests = requests;
    use->prepared = prepared;
    use->producer_request = producer;
    return true;
}
