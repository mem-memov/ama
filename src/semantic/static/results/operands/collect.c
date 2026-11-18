/**
 * @file
 * @brief Collect original lexical input terms without preparing child bodies.
 *
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) preserves structural
 * child identities. [TPL-11](../../../../../spec/language/templates.md#tpl-11)
 * keeps this finite preparation separate from guarded definition expansion.
 */
#include <stdint.h>
#include "internal.h"
#include "../../bindings/normalize.h"

/** Normalize one finite local request list and retain each explicit input term.
 * Child source registration does not prepare, apply or evaluate that child.
 * Partial arrays/requests/sources remain with their existing owners on refusal.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) requires the same
 * caller environment for each original child, independently of later producers.
 */
bool static_result_collect_operands(StaticResultPlan *plan,
    const StaticResultSource *source, const StaticTermApplication **output) {
    StaticApplicationPlan *applications = plan->guards->applications;
    BindingApplication *requests;
    if (!binding_normalize_static(applications->bindings, NULL, source->expression, &requests)) {
        (void)static_result_dependencies(plan); return false;
    }
    if (requests->input_count > SIZE_MAX / sizeof(StaticTermOperand)) {
        return static_result_fail(plan, STATIC_RESULT_OUT_OF_MEMORY, source->expression->span,
            "Static source operand storage exceeds representable size");
    }
    StaticTermOperand *values = requests->input_count
        ? static_result_allocate(plan, requests->input_count * sizeof(*values)) : NULL;
    if (requests->input_count && values == NULL) { return false; }
    size_t count = 0;
    for (const BindingRequest *request = requests->first; request; request = request->next) {
        if (request->direction != BINDING_STATIC_INPUT || request->omitted) { continue; }
        const StaticResultSource *child;
        if (!static_result_source(plan, source->environment, request->syntax.static_binding->input, &child)) { return false; }
        values[count++] = (StaticTermOperand){.request = request, .value = child->value};
    }
    if (!static_term_application(applications->terms, applications->bindings, requests,
        source->environment, source->head, values, count, output)) {
        (void)static_result_dependencies(plan); return false;
    }
    return true;
}
