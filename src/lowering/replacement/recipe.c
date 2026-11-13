/** @file
 * @brief Choose exactly one typed replacement construction path and preserve explicit source lifetime.
 * [CALL-08](../../../spec/language/calls-and-results.md#call-08) evaluates supplied
 * sources in the initiating caller, defaults in the callee, and output receivers
 * only at their own ordered transfer. No recipe allocates or copies runtime values.
 */
#include <assert.h>
#include "internal.h"
/** Resolve a recipe from checked semantic operands; every failure leaves its arena prefix owned. */
static ReplacementPlan *recipe(Lowering *lowering, const SemanticStatement *statement,
    const SemanticBinding *binding, const StorageField *field, ReplacementContext context) {
    ReplacementPlan *result = plan_allocate(lowering->plan, sizeof(*result));
    if (result == NULL) { return NULL; }
    *result = (ReplacementPlan){.allocated_next = lowering->type->replacements, .context = context, .statement = statement, .binding = binding};
    lowering->type->replacements = result;
    if (context == REPLACE_ASSIGNMENT) {
        result->receiver = statement->access;
        result->source = statement->source;
        result->value_type = statement->access->symbol->type;
        result->form = statement->access->list ? (statement->access->reference ? STORAGE_REFERENCE_LIST : STORAGE_OWNING_LIST)
            : statement->access->reference ? STORAGE_REFERENCE : STORAGE_OWNED;
        result->transfer = statement->transfer;
    } else if (context == REPLACE_OUTPUT) {
        result->construction = REPLACEMENT_RETAINED_RESULT;
        result->receiver = binding->receiver;
        result->source = binding->source;
        result->field = binding->field;
        result->value_type = binding->type;
        result->form = binding->receiver->list ? (binding->receiver->reference ? STORAGE_REFERENCE_LIST : STORAGE_OWNING_LIST)
            : binding->receiver->reference ? STORAGE_REFERENCE : STORAGE_OWNED;
        result->transfer = binding->transfer;
        return result;
    } else {
        result->field = field->symbol;
        result->value_type = field->symbol->type;
        result->form = field->form;
        if (binding == NULL) {
            result->construction = REPLACEMENT_DECLARED_DEFAULT;
            result->transfer = TRANSFER_DEFAULT;
            return result;
        }
        result->source = binding->source;
        result->transfer = binding->transfer;
    }
    result->construction = result->transfer == TRANSFER_LITERAL ? REPLACEMENT_SUPPLIED_LITERAL : REPLACEMENT_SUPPLIED_SOURCE;
    return result;
}
/** Prepend the fixed failure-safe instruction sequence; selected sources are consumed without reevaluation. */
static ControlNode *phases(Lowering *lowering, const SemanticStatement *statement,
    const SemanticBinding *binding, const StorageField *field, ReplacementPlan *value, ControlNode *after) {
    ReplacementContext context = value->context;
    ControlOperation first = context == REPLACE_ASSIGNMENT ? CONTROL_SELECT_RECEIVER
        : context == REPLACE_PARAMETER ? CONTROL_INSTALL_PARAMETER
        : context == REPLACE_RESULT ? CONTROL_INITIALIZE_RESULT
        : context == REPLACE_METHOD_PARAMETER ? CONTROL_STAGE_METHOD_INPUT : CONTROL_TRANSFER_OUTPUT;
    const ControlOperation phases[] = {first, CONTROL_SELECT_SOURCE, CONTROL_PREFLIGHT_REPLACEMENT,
        CONTROL_PREPARE_REPLACEMENT, CONTROL_COMMIT_REPLACEMENT, CONTROL_RELEASE_VALUES};
    ControlNode *next = after;
    for (size_t index = sizeof(phases) / sizeof(*phases); index > 0; index--) {
        if (phases[index - 1] == CONTROL_SELECT_SOURCE && context != REPLACE_ASSIGNMENT) { continue; }
        next = lower_prepend(lowering, phases[index - 1], statement, binding, next);
        if (next == NULL) { return NULL; }
        next->replacement = value;
        if (phases[index - 1] == CONTROL_COMMIT_REPLACEMENT) { value->commit = next; }
        if (phases[index - 1] == CONTROL_RELEASE_VALUES) { value->last = next; }
        if (context == REPLACE_METHOD_PARAMETER) {
            next->field = NULL;
            next->value_type = value->value_type;
            next->value_form = value->form;
            next->position = value->position;
        } else if (field != NULL) {
            next->field = field->symbol;
            next->value_type = field->symbol->type;
            next->value_form = field->form;
            next->position = field->symbol->role_ordinal;
        }
    }
    value->first = next;
    return next;
}

ControlNode *lower_replacement(Lowering *lowering, const SemanticStatement *statement,
    const SemanticBinding *binding, const StorageField *field, ReplacementContext context, ControlNode *after) {
    assert(context != REPLACE_METHOD_PARAMETER);
    ReplacementPlan *value = recipe(lowering, statement, binding, field, context);
    return value == NULL ? NULL : phases(lowering, statement, binding, field, value, after);
}

ControlNode *lower_method_parameter(Lowering *lowering, const SemanticStatement *statement,
    const SemanticListField *intrinsic, size_t position, const SemanticBinding *binding, ControlNode *after) {
    ReplacementPlan *value = plan_allocate(lowering->plan, sizeof(*value));
    if (value == NULL) { return NULL; }
    *value = (ReplacementPlan){.allocated_next = lowering->type->replacements, .context = REPLACE_METHOD_PARAMETER,
        .statement = statement, .binding = binding, .intrinsic = intrinsic, .position = position,
        .source = binding == NULL ? NULL : binding->source,
        .value_type = intrinsic->element_reference ? statement->list_interface.element : statement->list_interface.integer,
        .form = intrinsic->element_reference ? STORAGE_REFERENCE : STORAGE_OWNED,
        .transfer = binding == NULL ? TRANSFER_DEFAULT : binding->transfer,
        .construction = binding == NULL ? REPLACEMENT_DECLARED_DEFAULT
            : binding->transfer == TRANSFER_LITERAL ? REPLACEMENT_SUPPLIED_LITERAL : REPLACEMENT_SUPPLIED_SOURCE};
    lowering->type->replacements = value;
    return phases(lowering, statement, binding, NULL, value, after);
}
