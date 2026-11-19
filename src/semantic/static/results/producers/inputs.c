/** @file
 * @brief Build exact producer input maps from explicit expressions and deterministic omitted-input holes.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) retains every original input.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) preserves lexical child/formal identity.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns partial arrays and terms.
 */
#include <stdint.h>
#include "internal.h"

/** Actual normalized slot identity, never matching descriptive local spelling,
 * selects a supplied input. Normalization already rejects duplicate labels.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08).
 */
static const BindingRequest *request_for(const BindingApplication *requests, const StaticSymbol *formal) {
    for (const BindingRequest *request = requests->first; request; request = request->next) {
        if (request->direction == BINDING_STATIC_INPUT && request->slot->static_symbol == formal) { return request; }
    }
    return NULL;
}

/** A finite map visits each actual formal once and searches finite named
 * requests, O(P(P+I)) including metadata ordinal lookup, O(P) owned storage.
 * Expression registration binds terms without descending into source bodies.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) forbids speculative producer evaluation.
 */
bool static_result_producer_inputs(StaticResultPlan *plan, const StaticResultSource *source,
    const BindingApplication *requests, const StaticTermCapture **output) {
    size_t count = source->producer->parameter_count;
    if (count > SIZE_MAX / sizeof(StaticTermCapture)) {
        return static_result_fail(plan, STATIC_RESULT_OUT_OF_MEMORY, source->expression->span,
            "Static producer input map exceeds representable storage");
    }
    StaticTermCapture *map = count ? static_result_allocate(plan, count * sizeof(*map)) : NULL;
    if (count && !map) { return false; }
    for (size_t index = 0; index < count; index++) {
        const StaticSymbol *formal = static_symbol_at(source->producer, STATIC_PARAMETER, index);
        const BindingRequest *request = request_for(requests, formal);
        const StaticTerm *value;
        if (request) {
            const StaticResultSource *child;
            if (!static_result_source(plan, source->environment, request->syntax.static_binding->input, &child)) { return false; }
            value = child->value;
        } else if (!static_term_inference(plan->guards->applications->terms, source->environment,
            formal, NULL, source->expression, &value)) {
            (void)static_result_dependencies(plan); return false;
        }
        map[index] = (StaticTermCapture){.formal = formal, .value = value};
    }
    *output = map;
    return true;
}
