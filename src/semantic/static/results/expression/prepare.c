/** @file
 * @brief Prepare one source operator with lexical child values and unchanged skip provenance.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) substitutes scoped structure.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) separates input and output actions.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns partial preparation.
 */
#include <stdint.h>
#include "../internal.h"
#include "../../bindings/normalize.h"

/** Publicly retained work identity must be checked before its payload is read.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves original uses.
 */
bool static_result_has_use(const StaticResultPlan *plan, const StaticResultUse *candidate) {
    for (const StaticResultUse *use = plan->first_use; use; use = use->next) {
        if (use == candidate) { return true; }
    }
    return false;
}

/** Normalize without an invented actual interface: T177 later rebinds original
 * requests once the independently known head supplies its origin. Each explicit
 * child receives a use in the same lexical context, never the producer context.
 * This operation does not descend, solve, execute, or publish selected outputs.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) preserves lexical scope.
 */
bool static_result_prepare(StaticResultPlan *plan, StaticResultUse *use) {
    if (!static_result_dependencies(plan)) { return false; }
    if (!static_result_has_use(plan, use)) {
        return static_result_fail(plan, STATIC_RESULT_INVALID, (SourceSpan){0}, "Invalid expression preparation handle");
    }
    if (use->prepared != NULL) { return true; }
    BindingApplication *requests;
    StaticApplicationPlan *applications = plan->guards->applications;
    if (!binding_normalize_static(applications->bindings, NULL, use->source->expression, &requests)) {
        (void)static_result_dependencies(plan); return false;
    }
    if (requests->input_count > SIZE_MAX / sizeof(StaticTermOperand)) {
        return static_result_fail(plan, STATIC_RESULT_OUT_OF_MEMORY, use->source->expression->span,
            "Static source operand storage exceeds representable size");
    }
    StaticTermOperand *operands = NULL;
    if (requests->input_count != 0) {
        operands = static_result_allocate(plan, requests->input_count * sizeof(*operands));
        if (operands == NULL) { return false; }
    }
    size_t count = 0;
    for (const BindingRequest *request = requests->first; request; request = request->next) {
        if (request->direction != BINDING_STATIC_INPUT || request->omitted) { continue; }
        StaticResultUse *child;
        if (!static_result_use(plan, use->context, request->syntax.static_binding->input,
            use->definition_path, &child)) { return false; }
        operands[count++] = (StaticTermOperand){.request = request, .value = child->source->value};
    }
    const StaticTermApplication *prepared;
    if (!static_term_application(applications->terms, applications->bindings, requests,
        use->source->environment, use->source->head, operands, count, &prepared)) {
        (void)static_result_dependencies(plan); return false;
    }
    use->prepared = prepared;
    return true;
}
