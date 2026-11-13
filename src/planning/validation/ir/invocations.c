/** @file
 * @brief Validate resolved entry preparation and depth-first result action continuations without following body cycles.
 * [CALL-08](../../../../spec/language/calls-and-results.md#call-08) fixes source,
 * installation and receiver order; [CALL-09](../../../../spec/language/calls-and-results.md#call-09)
 * completes a selected subtree before its next sibling and retires body returns first.
 */
#include "internal.h"
#include "semantic/lists/interface.h"
/** Require a unique exact operation site; allocation order cannot disambiguate duplicates. */
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
/** Compare one explicit phase without dereferencing or following an unrecognized runtime target. */
static bool matches(const ControlNode *node, const SemanticStatement *statement,
    ControlOperation operation, const SemanticBinding *binding) {
    return node != NULL && node->statement == statement && node->operation == operation && node->binding == binding;
}
/** Ordinary source selections precede every interface reset; literals are constructed at their own installation. */
static const ControlNode *sources(const SemanticStatement *statement, const ControlNode *node) {
    for (const SemanticBinding *binding = statement->bindings; binding != NULL; binding = binding->next) {
        if (binding->syntax->kind != AST_INPUT || binding->syntax->omitted || binding->transfer == TRANSFER_LITERAL) { continue; }
        if (!matches(node, statement, CONTROL_EVALUATE_INPUT, binding)) { return NULL; }
        node = node->next;
    }
    return node;
}
/** Validate the finite installation prefix, then use the explicit return continuation rather than traversing the callee body. */
static const ControlNode *entry_outputs(const SemanticStatement *statement, const ControlNode *node) {
    AstCallMode mode = statement->syntax->as.call.mode;
    if (mode == AST_ENTRY_CALL) {
        node = sources(statement, node);
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
        || !matches(node->next, statement, mode == AST_RESUME_CALL ? CONTROL_RESUME_INSTANCE : CONTROL_ENTER_INSTANCE, NULL)) { return NULL; }
    const ControlNode *outputs = node->resume;
    return matches(outputs, statement, CONTROL_RETIRE_RETURN, NULL) ? outputs->next : NULL;
}
/** Intrinsic methods use typed transient inputs and explicit preflight/prepare/commit, never a synthetic module return. */
static const ControlNode *method_outputs(const SemanticStatement *statement, const ControlNode *node) {
    node = sources(statement, node);
    for (size_t position = 0; semantic_list_position(statement->list_method, false, position) != NULL; position++) {
        if (node == NULL || node->operation != CONTROL_STAGE_METHOD_INPUT || node->statement != statement
            || node->replacement == NULL || node->replacement->context != REPLACE_METHOD_PARAMETER
            || node->replacement->position != position) { return NULL; }
        node = node->replacement->last->next;
    }
    if (!matches(node, statement, CONTROL_LIST_METHOD_PREFLIGHT, NULL)) { return NULL; }
    node = node->next;
    if (!matches(node, statement, CONTROL_LIST_METHOD_PREPARE, NULL)) { return NULL; }
    node = node->next;
    return matches(node, statement, CONTROL_LIST_METHOD_COMMIT, NULL) ? node->next : NULL;
}
/** Check source-ordered actions using a child's explicit completion edge rather than following its runtime body. */
static const ControlNode *actions(const SemanticStatement *statement, const PlannedChain *chain, const ControlNode *node) {
    for (const SemanticOutputAction *action = statement->actions; action != NULL; action = action->next) {
        if (action->binding != NULL) {
            if (action->binding->syntax->omitted) { continue; }
            if (!matches(node, statement, CONTROL_TRANSFER_OUTPUT, action->binding) || node->replacement == NULL) { return NULL; }
            node = node->replacement->last->next;
        } else {
            if (!matches(node, action->operation, CONTROL_ACTION_NEXT, NULL) || node->chain != chain
                || node->next == NULL || node->resume == NULL) { return NULL; }
            node = node->resume;
        }
    }
    return node;
}
/** Every source call belongs to exactly one initiating chain; child action markers supply its normal subtree continuation. */
static bool invocation(const PlannedType *type, const SemanticStatement *statement) {
    AstCallMode mode = statement->syntax->as.call.mode;
    const ControlNode *node = NULL;
    const ControlNode *action = statement->enclosing_operation == NULL ? NULL : site(type, statement, CONTROL_ACTION_NEXT, NULL);
    if (statement->enclosing_operation != NULL && action == NULL) { return false; }
    const PlannedChain *chain = NULL;
    if (statement->operation_kind == SEMANTIC_LIST_METHOD_OPERATION) {
        node = action == NULL ? NULL : action->next;
        chain = action == NULL ? NULL : action->chain;
        if (node == NULL || chain == NULL) { return false; }
        node = method_outputs(statement, node);
    } else if (mode != AST_CONSTANT_READ) {
        const ControlNode *acquire = site(type, statement, statement->operation_kind == SEMANTIC_LIST_SERVICE_OPERATION
            ? CONTROL_ACQUIRE_LIST : CONTROL_ACQUIRE_INSTANCE, NULL);
        if (acquire == NULL || acquire->chain == NULL) { return false; }
        chain = acquire->chain;
        if (action != NULL) {
            const ControlNode *selected = action->next;
            if (!matches(selected, statement, CONTROL_SELECT_RESULT, NULL) || selected->next != acquire
                || selected->alternative != acquire->alternative || selected->chain != chain) { return false; }
        }
        node = acquire->next;
        if (statement->operation_kind == SEMANTIC_INSTANCE_OPERATION && (mode == AST_ENTRY_CALL || mode == AST_RESUME_CALL)) {
            node = entry_outputs(statement, node);
        }
    } else {
        for (const PlannedChain *candidate = type->chains; candidate != NULL; candidate = candidate->next) {
            if (candidate->root == statement) { if (chain != NULL) { return false; } chain = candidate; }
        }
        if (chain == NULL) { return false; }
        if (statement->actions != NULL) {
            const SemanticOutputAction *first = statement->actions;
            while (first != NULL && first->binding != NULL && first->binding->syntax->omitted) { first = first->next; }
            if (first != NULL) { node = first->binding != NULL ? site(type, statement, CONTROL_TRANSFER_OUTPUT, first->binding)
                : site(type, first->operation, CONTROL_ACTION_NEXT, NULL); }
        }
        if (node == NULL) { node = chain->exit; }
    }
    if (node == NULL) { return false; }
    node = actions(statement, chain, node);
    if (node == NULL) { return false; }
    if (chain->header != NULL) {
        return node == (chain->header->statement->condition_owner == statement ? chain->header->test : chain->header->after);
    }
    const ControlNode *after = action == NULL ? chain->exit : action->resume;
    if (mode == AST_CONSTANT_READ) { return node == after; }
    return matches(node, statement, CONTROL_RELEASE_TARGET, NULL) && node->chain == chain && node->next == after;
}
bool plan_ir_invocations(ProgramPlan *plan, const PlannedType *type) {
    for (const SemanticStatement *statement = type->type->statements; statement != NULL; statement = statement->next) {
        if (statement->syntax->kind == AST_CALL && !invocation(type, statement)) {
            return plan_fail(plan, statement->syntax->span, "Invocation IR violates selection, preparation or depth-first action order");
        }
    }
    return true;
}
