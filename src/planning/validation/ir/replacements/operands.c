/** @file
 * @brief Compare replacement recipes with their original resolved source/receiver and construction path.
 * [CALL-08](../../../../../spec/language/calls-and-results.md#call-08) distinguishes
 * caller sources, callee defaults and retained interface results without snapshots.
 */
#include "internal.h"
#include "semantic/lists/interface.h"
/** Derive ownership/cardinality from an already resolved storage view. Component contract: [CALL-08](../../../../../spec/language/calls-and-results.md#call-08). */
static StorageForm form(bool reference, bool list) {
    return list ? (reference ? STORAGE_REFERENCE_LIST : STORAGE_OWNING_LIST)
        : reference ? STORAGE_REFERENCE : STORAGE_OWNED;
}
/** Reject foreign statement/binding addresses before dereferencing recipe operands. Component contract: [CALL-08](../../../../../spec/language/calls-and-results.md#call-08). */
static bool identities(const PlannedType *type, const ReplacementPlan *recipe) {
    bool found = false;
    for (const SemanticStatement *statement = type->type->statements; statement != NULL; statement = statement->next) {
        if (recipe->statement == statement) { found = true; break; }
    }
    if (!found) { return false; }
    if (recipe->binding == NULL) { return true; }
    for (const SemanticBinding *binding = recipe->statement->bindings; binding != NULL; binding = binding->next) {
        if (recipe->binding == binding) { return !binding->syntax->omitted; }
    }
    return false;
}
/** Resolve the one expected supplied input by exact callee field identity. Component contract: [CALL-08](../../../../../spec/language/calls-and-results.md#call-08). */
static const SemanticBinding *supplied(const SemanticStatement *statement, const SemanticSymbol *field) {
    for (const SemanticBinding *binding = statement->bindings; binding != NULL; binding = binding->next) {
        if (binding->field == field && binding->syntax->kind == AST_INPUT && !binding->syntax->omitted) { return binding; }
    }
    return NULL;
}
/** Return the matched declaration only after comparing field identity within this exact callee. Component contract: [CALL-08](../../../../../spec/language/calls-and-results.md#call-08). */
static const SemanticSymbol *field_for(const ReplacementPlan *recipe) {
    if (recipe->statement->namespace_type == NULL) { return NULL; }
    for (const SemanticSymbol *field = recipe->statement->namespace_type->first; field != NULL; field = field->next) {
        if (field == recipe->field) { return field; }
    }
    return NULL;
}
bool plan_ir_replacement_operands(const PlannedType *type, const ReplacementPlan *recipe) {
    if (!identities(type, recipe)) { return false; }
    const SemanticStatement *statement = recipe->statement;
    const SemanticBinding *binding = recipe->binding;
    const SemanticSource *source = NULL;
    const SemanticAccess *receiver = NULL;
    SemanticType *value_type = NULL;
    SemanticTransfer transfer = TRANSFER_DEFAULT;
    ReplacementConstruction construction = REPLACEMENT_DECLARED_DEFAULT;
    StorageForm storage = STORAGE_OWNED;
    if (recipe->context == REPLACE_ASSIGNMENT) {
        if (statement->syntax->kind != AST_ASSIGNMENT || binding != NULL || recipe->field != NULL) { return false; }
        receiver = statement->access;
        source = statement->source;
        value_type = receiver->symbol->type;
        storage = form(receiver->reference, receiver->list);
        transfer = statement->transfer;
        construction = transfer == TRANSFER_LITERAL ? REPLACEMENT_SUPPLIED_LITERAL : REPLACEMENT_SUPPLIED_SOURCE;
    } else if (recipe->context == REPLACE_OUTPUT) {
        if (statement->syntax->kind != AST_CALL || binding == NULL || binding->syntax->kind == AST_INPUT
            || recipe->field != binding->field) { return false; }
        receiver = binding->receiver;
        source = binding->source;
        value_type = receiver->symbol->type;
        storage = form(receiver->reference, receiver->list);
        transfer = binding->transfer;
        construction = REPLACEMENT_RETAINED_RESULT;
    } else if (recipe->context == REPLACE_METHOD_PARAMETER) {
        if (statement->operation_kind != SEMANTIC_LIST_METHOD_OPERATION || recipe->field != NULL
            || recipe->intrinsic == NULL || recipe->intrinsic != semantic_list_position(statement->list_method, false, recipe->position)) { return false; }
        const SemanticBinding *expected = NULL;
        for (const SemanticBinding *input = statement->bindings; input != NULL; input = input->next) {
            if (input->syntax->kind == AST_INPUT && input->position == recipe->position && !input->syntax->omitted) { expected = input; break; }
        }
        if (binding != expected) { return false; }
        value_type = recipe->intrinsic->element_reference ? statement->list_interface.element : statement->list_interface.integer;
        storage = recipe->intrinsic->element_reference ? STORAGE_REFERENCE : STORAGE_OWNED;
        if (binding != NULL) {
            source = binding->source;
            transfer = binding->transfer;
            construction = transfer == TRANSFER_LITERAL ? REPLACEMENT_SUPPLIED_LITERAL : REPLACEMENT_SUPPLIED_SOURCE;
        }
    } else if (recipe->context == REPLACE_PARAMETER || recipe->context == REPLACE_RESULT) {
        const SemanticSymbol *field = field_for(recipe);
        if (statement->syntax->kind != AST_CALL || statement->syntax->as.call.mode != AST_ENTRY_CALL
            || field == NULL || field->role != (recipe->context == REPLACE_PARAMETER ? SYMBOL_PARAMETER : SYMBOL_RESULT)
            || binding != (recipe->context == REPLACE_PARAMETER ? supplied(statement, field) : NULL)) { return false; }
        value_type = field->type;
        storage = form(field->reference, field->list);
        if (binding != NULL) {
            source = binding->source;
            transfer = binding->transfer;
            construction = transfer == TRANSFER_LITERAL ? REPLACEMENT_SUPPLIED_LITERAL : REPLACEMENT_SUPPLIED_SOURCE;
        }
    } else { return false; }
    if (recipe->context != REPLACE_METHOD_PARAMETER && recipe->intrinsic != NULL) { return false; }
    return recipe->source == source && recipe->receiver == receiver && recipe->value_type == value_type
        && recipe->form == storage && recipe->transfer == transfer && recipe->construction == construction;
}
