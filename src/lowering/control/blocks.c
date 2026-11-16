/**
 * @file
 * @brief Lower source-order blocks through reverse work records and explicit terminal edges.
 *
 * [GEN-02](../../../spec/implementation/compiler.md#gen-02)
 * Stable arena slots allow child entry patching after parent construction without recursive C frames.
 */
#include "internal.h"

/** Plan-owned reverse syntax link used to prepend instructions without reversing source execution order. Component contract: [GEN-02](../../../spec/implementation/compiler.md#gen-02). */
typedef struct ReverseNode {
    struct ReverseNode *next;
    const AstNode *syntax;
} ReverseNode;

bool lower_block(Lowering *lowering, AstBlock block, ControlNode *after,
    ControlNode *break_target, ControlNode **destination) {
    LowerTask *task = plan_allocate(lowering->plan, sizeof(*task));
    if (task == NULL) {
        return false;
    }
    *task = (LowerTask){.next = lowering->pending, .kind = LOWER_BLOCK_TASK, .block = block,
        .after = after, .break_target = break_target, .destination = destination};
    lowering->pending = task;
    return true;
}

/** Reverse finite AST node links, prepend lowered statements then publish this queued block entry. All reversal scratch remains plan-owned.
 * Component contract: [GEN-02](../../../spec/implementation/compiler.md#gen-02). */
static bool process(Lowering *lowering, const LowerTask *task) {
    if (task->kind == LOWER_OPERATION_TASK) {
        lowering->chain = task->chain;
        ControlNode *entry = lower_call(lowering, task->operation, task->after, task->break_target);
        lowering->chain = NULL;
        if (entry == NULL) { return false; }
        *task->destination = entry;
        return true;
    }
    lowering->chain = NULL;
    ReverseNode *reverse = NULL;
    for (const AstNode *node = task->block.first; node != NULL; node = node->next) {
        ReverseNode *frame = plan_allocate(lowering->plan, sizeof(*frame));
        if (frame == NULL) {
            return false;
        }
        *frame = (ReverseNode){reverse, node};
        reverse = frame;
    }
    ControlNode *next = task->after;
    for (; reverse != NULL; reverse = reverse->next) {
        const SemanticStatement *statement = plan_statement(lowering->type, reverse->syntax);
        next = lower_statement(lowering, statement, next, task->break_target);
        if (next == NULL) {
            return false;
        }
    }
    *task->destination = next;
    return true;
}

/** Allocate completion/yield and capture-before-cleanup fatal terminal graph, requiring all endpoints before normal body lowering.
 * Component contract: [GEN-02](../../../spec/implementation/compiler.md#gen-02). */
static bool endpoints(ProgramPlan *plan, PlannedType *type) {
    type->complete = plan_control_node(plan, type, CONTROL_COMPLETE, NULL);
    type->yield = plan_control_node(plan, type, CONTROL_YIELD, NULL);
    ControlNode *capture = plan_control_node(plan, type, CONTROL_CAPTURE_ERROR, NULL);
    ControlNode *cleanup = plan_control_node(plan, type, CONTROL_CLEANUP_FAILURE, NULL);
    ControlNode *failed = plan_control_node(plan, type, CONTROL_FAILURE, NULL);
    if (type->complete == NULL || type->yield == NULL || capture == NULL || cleanup == NULL || failed == NULL) {
        return false;
    }
    capture->next = cleanup;
    cleanup->next = failed;
    type->failure = capture;
    return true;
}

bool plan_control(ProgramPlan *plan, PlannedType *type) {
    if (!endpoints(plan, type)) {
        return false;
    }
    Lowering lowering = {.plan = plan, .type = type};
    if (!lower_loop_inventory(&lowering)) { return false; }
    if (type->type->module->syntax.sections.kind == MODULE_NATIVE) {
        type->entry = lower_prepend(&lowering, CONTROL_NATIVE_ENTRY, NULL, NULL, type->complete);
        return type->entry != NULL;
    }
    if (!lower_block(&lowering, type->type->module->syntax.body, type->complete, NULL, &type->entry)) {
        return false;
    }
    while (lowering.pending != NULL) {
        LowerTask *task = lowering.pending;
        lowering.pending = task->next;
        if (!process(&lowering, task)) {
            return false;
        }
    }
    return lower_finish_loops(&lowering);
}
