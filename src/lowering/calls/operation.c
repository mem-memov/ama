/**
 * @file
 * @brief Compose acquisition, input preparation, return connection and ordered output phases.
 *
 * [CALL-08](../../../spec/language/calls-and-results.md#call-08)
 * Only acquisition refusal selects the attached alternative; fatal failure and output continuation remain separate edges.
 */
#include "internal.h"
#include "planning/storage/frames/internal.h"

/** For active modes, create entry/resume and explicit return/output continuation; retained/shared reads return their output graph directly.
 */
static ControlNode *execute(Lowering *lowering, const SemanticStatement *statement,
    ControlNode *outputs) {
    AstCallMode mode = statement->syntax->as.call.mode;
    if (mode == AST_CONSTANT_READ || mode == AST_RETAINED_READ) {
        return outputs;
    }
    if (statement->operation_kind != SEMANTIC_INSTANCE_OPERATION) { return outputs; }
    outputs = lower_prepend(lowering, CONTROL_RETIRE_RETURN, statement, NULL, outputs);
    if (outputs == NULL) { return NULL; }
    outputs->chain = lowering->chain;
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
    ControlNode *next) {
    ControlNode *node = lower_prepend(lowering, CONTROL_ACQUIRE_INSTANCE, statement, NULL, next);
    if (node == NULL) {
        return NULL;
    }
    node->chain = lowering->chain;
    node->alternative = lower_refusal(lowering, statement);
    if (node->alternative == NULL) { return NULL; }
    if (statement->operation_kind == SEMANTIC_LIST_SERVICE_OPERATION) { node->operation = CONTROL_ACQUIRE_LIST; }
    return node;
}

/** Build one link while preserving its initiating root and header/output continuation; selected children are queued separately. */
static ControlNode *operation(Lowering *lowering, const SemanticStatement *statement,
    ControlNode *after) {
    bool static_read = statement->syntax->as.call.mode == AST_CONSTANT_READ;
    PlannedLoop *header = lowering->chain->header;
    ControlNode *outputs = header != NULL ? (header->statement->condition_owner == statement ? header->test : header->after)
        : static_read ? after : lower_prepend(lowering, CONTROL_RELEASE_TARGET, statement, NULL, after);
    if (outputs != NULL && outputs->operation == CONTROL_RELEASE_TARGET) { outputs->chain = lowering->chain; }
    if (outputs == NULL || (outputs = lower_actions(lowering, statement, outputs)) == NULL) {
        return NULL;
    }
    if (statement->operation_kind == SEMANTIC_LIST_METHOD_OPERATION) {
        ControlNode *refusal = lower_refusal(lowering, statement);
        return refusal == NULL ? NULL : lower_list_method(lowering, statement, outputs, refusal);
    }
    ControlNode *next = execute(lowering, statement, outputs);
    if (next == NULL) {
        return NULL;
    }
    if (statement->syntax->as.call.mode == AST_ENTRY_CALL
        && statement->operation_kind == SEMANTIC_INSTANCE_OPERATION) {
        ReverseBinding *reverse = lower_reverse_bindings(lowering, statement);
        if (lowering->plan->status != PLAN_OK) { return NULL; }
        next = lower_install(lowering, statement, next);
        if (next != NULL) { next = lower_prepend(lowering, CONTROL_PREFLIGHT_INPUTS, statement, NULL, next); }
        if (next == NULL || (next = lower_inputs(lowering, statement, reverse, next)) == NULL) {
            return NULL;
        }
    }
    next = static_read ? next : acquire(lowering, statement, next);
    if (next != NULL && statement->enclosing_operation != NULL) {
        ControlNode *selected = lower_prepend(lowering, CONTROL_SELECT_RESULT, statement, NULL, next);
        if (selected == NULL) { return NULL; }
        selected->alternative = next->alternative;
        selected->chain = lowering->chain;
        next = selected;
    }
    return next;
}

ControlNode *lower_call(Lowering *lowering, const SemanticStatement *statement,
    ControlNode *after, ControlNode *break_target) {
    PlannedChain *previous = lowering->chain;
    if (previous == NULL) {
        PlannedChain *chain = plan_allocate(lowering->plan, sizeof(*chain));
        if (chain == NULL) { return NULL; }
        *chain = (PlannedChain){.next = lowering->type->chains, .root = statement,
            .root_frame = plan_storage_frame(lowering->type, statement), .exit = after, .break_target = break_target};
        lowering->type->chains = chain;
        lowering->chain = chain;
    }
    ControlNode *entry = operation(lowering, statement, after);
    lowering->chain = previous;
    return entry;
}
