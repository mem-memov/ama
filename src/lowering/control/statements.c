#include "internal.h"

static ControlNode *control(Lowering *lowering, const SemanticStatement *statement,
    ControlNode *after, ControlNode *break_target) {
    const AstNode *node = statement->syntax;
    bool loop = node->kind == AST_LOOP;
    ControlNode *condition = lower_prepend(lowering, CONTROL_CONDITION, statement, NULL, after);
    if (condition == NULL) {
        return NULL;
    }
    condition->alternative = after;
    if (!lower_block(lowering, node->as.control.body, loop ? condition : after,
            loop ? after : break_target, &condition->next)
        || (!loop && !lower_block(lowering, node->as.control.alternative, after,
            break_target, &condition->alternative))) {
        return NULL;
    }
    return condition;
}

ControlNode *lower_statement(Lowering *lowering, const SemanticStatement *statement,
    ControlNode *after, ControlNode *break_target) {
    const AstNode *node = statement->syntax;
    switch (node->kind) {
        case AST_DECLARATION:
            return after;
        case AST_ASSIGNMENT:
            return lower_assignment(lowering, statement, after);
        case AST_CALL:
            return lower_call(lowering, statement, after, break_target);
        case AST_BRANCH:
        case AST_LOOP:
            return control(lowering, statement, after, break_target);
        case AST_BREAK:
            if (break_target == NULL) {
                plan_fail(lowering->plan, node->span, "Loop exit ^ requires an enclosing loop");
            }
            return break_target;
        case AST_RETURN:
            return lowering->type->complete;
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
