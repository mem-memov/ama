/**
 * @file
 * @brief Normalize one static expression's own input/output requests without descending into nested owners.
 *
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) gives every
 * expression level its own mode and counters, leaves unknown origins pending,
 * and preserves actual residual snapshots throughout all requests.
 */
#include <assert.h>
#include "../normalize.h"
#include "../internal.h"

/** Preserve complete structural requests and borrowed operands. Nested input
 * expressions remain their own later applications; no textual substitution or
 * selected-output execution occurs during positional/name resolution.
 * Contract: [TPL-12](../../../../../spec/language/static-results.md#tpl-12).
 */
bool binding_normalize_static(BindingPlan *plan, const BindingInterface *view,
    const AstTypeExpression *expression, BindingApplication **output) {
    if (plan->status != BINDING_OK) { return false; }
    assert(expression != NULL && output != NULL);
    BindingApplication *application = NULL;
    if (!binding_application_begin(plan, view, expression->head, BINDING_STATIC_APPLICATION, &application)) { return false; }
    application->expression = expression;
    for (const AstStaticBinding *binding = expression->first; binding != NULL; binding = binding->next) {
        BindingRequest request = {.span = binding->span, .marker = binding->marker, .label = binding->label,
            .direction = binding->kind == AST_STATIC_INPUT ? BINDING_STATIC_INPUT : BINDING_STATIC_OUTPUT,
            .named = true, .syntax.static_binding = binding};
        if (!binding_request_append(plan, application, request)) { return false; }
    }
    binding_application_publish(plan, application, output);
    return true;
}
