/**
 * @file
 * @brief Compose acquisition, input preparation, return connection and ordered output phases.
 *
 * [CALL-08](../../../spec/language/calls-and-results.md#call-08)
 * Only acquisition refusal selects the attached alternative; fatal failure and output continuation remain separate edges.
 */
#include "internal.h"

/** For active modes, create entry/resume and explicit return/output continuation; retained/shared reads return their output graph directly.
 */
static ControlNode *execute(Lowering *lowering, const SemanticStatement *statement,
    ControlNode *outputs) {
    AstCallMode mode = statement->syntax->as.call.mode;
    if (mode == AST_CONSTANT_READ || mode == AST_RETAINED_READ) {
        return outputs;
    }
    ControlNode *entry = lower_prepend(lowering, mode == AST_RESUME_CALL
        ? CONTROL_RESUME_INSTANCE : CONTROL_ENTER_INSTANCE, statement, NULL, NULL);
    if (entry == NULL) {
        return NULL;
    }
    entry->callee = program_plan_type(lowering->plan, statement->namespace_type);
    ControlNode *connection = lower_prepend(lowering, CONTROL_SET_RETURN, statement, NULL, entry);
    if (connection != NULL) {
        connection->resume = outputs;
        connection->callee = entry->callee;
    }
    return connection;
}

/** Create exact target acquisition plus fatal/default or lexical refusal graph, retaining handler break context separately.
 */
static ControlNode *acquire(Lowering *lowering, const SemanticStatement *statement,
    ControlNode *next, ControlNode *after, ControlNode *break_target) {
    ControlNode *node = lower_prepend(lowering, CONTROL_ACQUIRE_INSTANCE, statement, NULL, next);
    if (node == NULL) {
        return NULL;
    }
    node->alternative = lowering->type->failure;
    if (statement->syntax->as.call.has_alternative) {
        ControlNode *snapshot = lower_prepend(lowering, CONTROL_REFUSAL_SNAPSHOT, statement, NULL, after);
        if (snapshot == NULL || !lower_block(lowering, statement->syntax->as.call.alternative,
                after, break_target, &snapshot->next)) {
            return NULL;
        }
        node->alternative = snapshot;
    }
    return node;
}

ControlNode *lower_call(Lowering *lowering, const SemanticStatement *statement,
    ControlNode *after, ControlNode *break_target) {
    bool static_read = statement->syntax->as.call.mode == AST_CONSTANT_READ;
    ReverseBinding *reverse = lower_reverse_bindings(lowering, statement);
    if (lowering->plan->status != PLAN_OK) {
        return NULL;
    }
    ControlNode *outputs = static_read ? after
        : lower_prepend(lowering, CONTROL_RELEASE_TARGET, statement, NULL, after);
    if (outputs == NULL || (outputs = lower_outputs(lowering, statement, reverse, outputs)) == NULL) {
        return NULL;
    }
    ControlNode *next = execute(lowering, statement, outputs);
    if (next == NULL) {
        return NULL;
    }
    if (statement->syntax->as.call.mode == AST_ENTRY_CALL) {
        next = lower_install(lowering, statement, next);
        if (next == NULL || (next = lower_inputs(lowering, statement, reverse, next)) == NULL) {
            return NULL;
        }
    }
    return static_read ? next : acquire(lowering, statement, next, after, break_target);
}
