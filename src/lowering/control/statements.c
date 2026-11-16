/**
 * @file
 * @brief Lower retained statement/control shapes to explicit successor, break and resume edges.
 *
 * [GEN-02](../../../spec/implementation/compiler.md#gen-02)
 * [FLOW-02](../../../spec/language/control-flow.md#flow-02) exits after one
 * successful check for parsed-empty loops. Delegate complete headers to lower_control
 * and [FLOW-05](../../../spec/language/lists.md#flow-05) traversal frames to lower_foreach;
 * a saved position denotes the resolved successor, including the recorded repeat
 * check when suspension is the last statement in a loop body.
 */
#include "internal.h"

ControlNode *lower_statement(Lowering *lowering, const SemanticStatement *statement,
    ControlNode *after, ControlNode *break_target) {
    const AstNode *node = statement->syntax;
    switch (node->kind) {
        case AST_FOREACH:
            return lower_foreach(lowering, statement, after);
        case AST_DECLARATION:
            return after;
        case AST_ASSIGNMENT:
            return lower_assignment(lowering, statement, after);
        case AST_CALL:
            return lower_call(lowering, statement, after, break_target);
        case AST_BRANCH:
        case AST_LOOP:
            return lower_control(lowering, statement, after, break_target);
        case AST_BREAK:
            if (break_target == NULL) {
                plan_fail(lowering->plan, node->span, "Loop exit ^ requires an enclosing loop");
            }
            return break_target;
        case AST_RETURN: {
            const SemanticStatement *scope = statement->enclosing_loop;
            while (scope != NULL && scope->syntax->kind != AST_FOREACH) { scope = scope->enclosing_loop; }
            if (scope == NULL) { return lowering->type->complete; }
            ControlNode *cleanup = lower_prepend(lowering, CONTROL_CLEANUP_SCOPES, statement, NULL, lowering->type->complete);
            if (cleanup != NULL) { cleanup->loop = lower_loop(lowering->type, scope); }
            return cleanup;
        }
        case AST_SUSPEND: {
            ControlNode *yield = lower_prepend(lowering, CONTROL_SAVE_POSITION, statement, NULL, lowering->type->yield);
            if (yield != NULL) {
                yield->resume = after;
            }
            return yield;
        }
    }
    return NULL;
}
