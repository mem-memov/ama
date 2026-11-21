/** @file
 * @brief Cache actual named producer request maps without creating a producer Type or entering its definitions.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) uses only named actual interfaces.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) preserves source environment keys.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) publishes complete preparation only.
 */
#include <stdint.h>
#include "internal.h"
#include "../../bindings/normalize.h"

/** Normalization identifies original input/export slots before any input-map
 * allocation. Cache identity is exact source/environment, not a work counter.
 * Partial child owners remain valid for cleanup after any later refusal.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves stable omitted variables.
 */
bool static_result_prepare_producer(StaticResultPlan *plan, const StaticResultSource *source,
    const StaticResultProducer **output) {
    if (!static_result_dependencies(plan)) { return false; }
    if (!output || !static_result_has_source(plan, source) || !source->producer) {
        return static_result_fail(plan, STATIC_RESULT_INVALID, (SourceSpan){0}, "Invalid named static producer source or receiver");
    }
    for (const StaticResultProducer *item = plan->first_producer_request; item; item = item->next) {
        if (item->source == source) { *output = item; return true; }
    }
    if (plan->producer_request_count == SIZE_MAX) {
        return static_result_fail(plan, STATIC_RESULT_OUT_OF_MEMORY, source->expression->span,
            "Static producer request count exceeds representable storage");
    }
    BindingPlan *bindings = plan->guards->applications->bindings;
    BindingInterface *interface;
    BindingApplication *requests;
    if (!binding_source_interface(bindings, source->producer, BINDING_MODULE_SURFACE, NULL, 0, &interface) ||
        !binding_normalize_static(bindings, interface, source->expression, &requests)) {
        (void)static_result_dependencies(plan); return false;
    }
    const StaticTermCapture *inputs;
    if (!static_result_producer_inputs(plan, source, requests, &inputs)) { return false; }
    StaticResultProducer *prepared = static_result_allocate(plan, sizeof(*prepared));
    if (!prepared) { return false; }
    *prepared = (StaticResultProducer){.next = plan->first_producer_request, .source = source,
        .requests = requests, .inputs = inputs, .input_count = source->producer->parameter_count};
    if (!static_result_require_producer_inputs(plan, prepared)) { return false; }
    plan->first_producer_request = prepared;
    plan->producer_request_count++;
    *output = prepared;
    return true;
}
