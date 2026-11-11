#include "internal.h"

static ControlNode *execute(Lowering *lowering, const SemanticStatement *statement,
    ControlNode *outputs, bool list) {
    AstCallMode mode = statement->syntax->as.call.mode;
    if (mode == AST_CONSTANT_READ || mode == AST_RETAINED_READ) {
        return outputs;
    }
    if (list) {
        return lower_prepend(lowering, CONTROL_LIST_ACTION, statement, NULL, outputs);
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

static ControlNode *acquire(Lowering *lowering, const SemanticStatement *statement,
    ControlNode *next, ControlNode *after, ControlNode *break_target, bool list) {
    ControlNode *node = lower_prepend(lowering, list ? CONTROL_ACQUIRE_LIST
        : CONTROL_ACQUIRE_INSTANCE, statement, NULL, next);
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
    bool list = statement->access != NULL && statement->access->list;
    ReverseBinding *reverse = lower_reverse_bindings(lowering, statement);
    if (lowering->plan->status != PLAN_OK) {
        return NULL;
    }
    ControlNode *outputs = static_read ? after
        : lower_prepend(lowering, CONTROL_RELEASE_TARGET, statement, NULL, after);
    if (outputs == NULL || (outputs = lower_outputs(lowering, statement, reverse, outputs)) == NULL) {
        return NULL;
    }
    ControlNode *next = execute(lowering, statement, outputs, list);
    if (next == NULL) {
        return NULL;
    }
    if (statement->syntax->as.call.mode == AST_ENTRY_CALL) {
        next = lower_install(lowering, statement, next, list);
        if (next == NULL || (next = lower_inputs(lowering, statement, reverse, next)) == NULL) {
            return NULL;
        }
    }
    return static_read ? next : acquire(lowering, statement, next, after, break_target, list);
}
