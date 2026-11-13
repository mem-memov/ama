/** @file
 * @brief Validate source-order selection, pre-reset conflict preflight, declaration installation and output text order.
 * [CALL-08](../../../../spec/language/calls-and-results.md#call-08) assigns these
 * distinct sequences; no earlier commit is rolled back when a later phase fails.
 */
#include "internal.h"
/** Require a unique operation site rather than selecting the first address matching a partial key. */
static const ControlNode *site(const PlannedType *type, const SemanticStatement *statement,
    ControlOperation operation, const SemanticBinding *binding) {
    const ControlNode *result = NULL;
    for (const ControlNode *node = type->controls; node != NULL; node = node->allocated_next) {
        if (node->statement != statement || node->operation != operation || node->binding != binding) { continue; }
        if (result != NULL) { return NULL; }
        result = node;
    }
    return result;
}
/** Check one exact instruction along the finite invocation prefix; runtime body backedges are never followed. */
static bool matches(const ControlNode *node, const SemanticStatement *statement,
    ControlOperation operation, const SemanticBinding *binding) {
    return node != NULL && node->statement == statement && node->operation == operation && node->binding == binding;
}
/** Validate entry preparation before following the explicit body return continuation, not the callee's executable graph. */
static const ControlNode *entry_outputs(const SemanticStatement *statement, const ControlNode *node) {
    AstCallMode mode = statement->syntax->as.call.mode;
    if (mode == AST_ENTRY_CALL) {
        for (const SemanticBinding *binding = statement->bindings; binding != NULL; binding = binding->next) {
            if (binding->syntax->kind != AST_INPUT || binding->syntax->omitted || binding->transfer == TRANSFER_LITERAL) { continue; }
            if (!matches(node, statement, CONTROL_EVALUATE_INPUT, binding)) { return NULL; }
            node = node->next;
        }
        if (!matches(node, statement, CONTROL_PREFLIGHT_INPUTS, NULL)) { return NULL; }
        node = node->next;
        for (const SemanticSymbol *field = statement->namespace_type->first; field != NULL; field = field->next) {
            if (field->role != SYMBOL_PARAMETER && field->role != SYMBOL_RESULT) { continue; }
            ControlOperation operation = field->role == SYMBOL_PARAMETER ? CONTROL_INSTALL_PARAMETER : CONTROL_INITIALIZE_RESULT;
            if (node == NULL || node->operation != operation || node->statement != statement
                || node->field != field || node->replacement == NULL) { return NULL; }
            node = node->replacement->last->next;
        }
    }
    if (!matches(node, statement, CONTROL_SET_RETURN, NULL)
        || !matches(node->next, statement, mode == AST_RESUME_CALL ? CONTROL_RESUME_INSTANCE : CONTROL_ENTER_INSTANCE, NULL)) {
        return NULL;
    }
    return node->resume;
}
/** Validate one complete ordinary call/read without following its body or the following caller statement. */
static bool invocation(const PlannedType *type, const SemanticStatement *statement) {
    AstCallMode mode = statement->syntax->as.call.mode;
    const ControlNode *node = NULL;
    if (mode != AST_CONSTANT_READ) {
        const ControlNode *acquire = site(type, statement, CONTROL_ACQUIRE_INSTANCE, NULL);
        if (acquire == NULL) { return false; }
        node = acquire->next;
        if (mode == AST_ENTRY_CALL || mode == AST_RESUME_CALL) {
            node = entry_outputs(statement, node);
            if (node == NULL) { return false; }
        }
    }
    for (const SemanticBinding *binding = statement->bindings; binding != NULL; binding = binding->next) {
        if (binding->syntax->kind == AST_INPUT || binding->syntax->omitted) { continue; }
        if (mode == AST_CONSTANT_READ && node == NULL) { node = site(type, statement, CONTROL_TRANSFER_OUTPUT, binding); }
        if (!matches(node, statement, CONTROL_TRANSFER_OUTPUT, binding) || node->replacement == NULL) { return false; }
        node = node->replacement->last->next;
    }
    return mode == AST_CONSTANT_READ || matches(node, statement, CONTROL_RELEASE_TARGET, NULL);
}
bool plan_ir_invocations(ProgramPlan *plan, const PlannedType *type) {
    for (const SemanticStatement *statement = type->type->statements; statement != NULL; statement = statement->next) {
        if (statement->syntax->kind != AST_CALL) { continue; }
        if (!invocation(type, statement)) {
            return plan_fail(plan, statement->syntax->span, "Invocation IR violates source selection, preparation or output action order");
        }
    }
    return true;
}
