/** @file
 * @brief Import prepared static inputs while preserving independent source-use records.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) separates values from uses.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns partial records.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) keeps static input and output requests distinct.
 */
#include <stdint.h>
#include "../application.h"
#include "../internal.h"

/** Only exact published static applications of a healthy plan may be borrowed.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) separates runtime requests.
 */
static bool registered_application(const BindingPlan *plan, const BindingApplication *application) {
    if (plan == NULL || plan->status != BINDING_OK) { return false; }
    for (const BindingApplication *item = plan->first_application; item; item = item->next) {
        if (item == application) { return item->kind == BINDING_STATIC_APPLICATION; }
    }
    return false;
}

/** The source expression must be registered in the supplied lexical owner.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) prevents scope capture.
 */
static bool owned_expression(const StaticTermEnvironment *environment, const BindingApplication *source) {
    for (const StaticOccurrence *item = environment->definition->first_occurrence; item; item = item->next) {
        if (item->expression == source->expression) { return true; }
    }
    return false;
}

/** Exact request identity avoids coupling argument order to prepared-array order.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) keeps substitutions structural.
 */
static const StaticTerm *operand_value(const StaticTermOperand *operands, size_t count,
    const BindingRequest *request) {
    for (size_t index = 0; index < count; index++) {
        if (operands[index].request == request) { return operands[index].value; }
    }
    return NULL;
}

/** Validate a bijection between explicit source inputs and supplied owned terms.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) forbids guessed missing values.
 */
static bool valid_operands(StaticTermStore *store, const BindingApplication *application,
    const StaticTermOperand *operands, size_t count) {
    if (count > application->input_count || (count != 0 && operands == NULL)) { return false; }
    size_t required = 0;
    for (const BindingRequest *request = application->first; request; request = request->next) {
        if (request->direction != BINDING_STATIC_INPUT || request->omitted) { continue; }
        required++;
        if (operand_value(operands, count, request) == NULL) { return false; }
    }
    if (required != count) { return false; }
    for (size_t index = 0; index < count; index++) {
        if (!static_term_has_value(store, operands[index].value)) { return false; }
        for (size_t previous = 0; previous < index; previous++) {
            if (operands[previous].request == operands[index].request) { return false; }
        }
    }
    return true;
}

/** Prepare complete input links before publishing provenance; failure never
 * mutates binding/source records. [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03).
 */
bool static_term_application(StaticTermStore *store, const BindingPlan *plan,
    const BindingApplication *source, const StaticTermEnvironment *environment,
    const StaticTerm *head, const StaticTermOperand *operands, size_t count,
    const StaticTermApplication **output) {
    if (store->status != STATIC_TERM_OK) { return false; }
    if (output == NULL || !registered_application(plan, source) ||
        !static_term_has_environment(store, environment) || !owned_expression(environment, source) ||
        !static_term_has_value(store, head) ||
        !valid_operands(store, source, operands, count)) {
        return static_term_invalid(store, (SourceSpan){0}, "Invalid prepared static application");
    }
    if (store->application_count == SIZE_MAX) { return static_term_memory_failure(store); }
    StaticTermApplication *application = static_term_allocate(store, sizeof(*application));
    if (application == NULL) { return false; }
    application->source = source;
    application->environment = environment;
    application->head = head;
    for (const BindingRequest *request = source->first; request; request = request->next) {
        if (request->direction != BINDING_STATIC_INPUT) { continue; }
        StaticTermInput *input = static_term_allocate(store, sizeof(*input));
        if (input == NULL) { return false; }
        input->request = request;
        input->value = request->omitted ? NULL : operand_value(operands, count, request);
        if (application->last != NULL) { application->last->next = input; }
        else { application->first = input; }
        application->last = input;
        application->input_count++;
    }
    if (store->last_application != NULL) { store->last_application->next = application; }
    else { store->first_application = application; }
    store->last_application = application;
    store->application_count++;
    *output = application;
    return true;
}
