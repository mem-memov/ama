/** @file
 * @brief Build fixed lexical condition entries and release complete header chains before body/exit.
 * [FLOW-04](../../../spec/language/control-flow.md#flow-04) repeats from the
 * first written operation and converts the terminal owned result while protected.
 * [FLOW-02](../../../spec/language/control-flow.md#flow-02) checks a parsed-empty
 * loop only once regardless of the successful boolean result.
 */
#include "lowering/control/internal.h"
#include "planning/storage/frames/internal.h"
/** Build one explicit caller-side header release path; both truth outcomes drop all header grants. */
static ControlNode *release(Lowering *lowering, const SemanticStatement *statement,
    PlannedChain *chain, ControlNode *after) {
    ControlNode *node = lower_prepend(lowering, CONTROL_RELEASE_CHAIN, statement, NULL, after);
    if (node != NULL) { node->chain = chain; node->loop = chain->header; }
    return node;
}
/** One root/terminal header owns its own repeat label; handler continuations deliberately retain the incoming break context. */
static ControlNode *header(Lowering *lowering, PlannedLoop *loop, ControlNode *after, ControlNode *break_target) {
    const SemanticStatement *statement = loop->statement;
    ControlNode *entry = lower_prepend(lowering, CONTROL_HEADER_ENTER, statement, NULL, NULL);
    ControlNode *condition = entry == NULL ? NULL : lower_prepend(lowering, CONTROL_CONDITION, statement, NULL, NULL);
    PlannedChain *chain = condition == NULL ? NULL : plan_allocate(lowering->plan, sizeof(*chain));
    if (chain == NULL) { return NULL; }
    *chain = (PlannedChain){.next = lowering->type->chains, .root = statement->loop_root,
        .root_frame = plan_storage_frame(lowering->type, statement->loop_root),
        .header = loop, .exit = after, .break_target = break_target};
    lowering->type->chains = chain;
    loop->header = chain;
    loop->entry = loop->repeat = entry;
    loop->test = condition;
    loop->exit = loop->after = after;
    entry->loop = condition->loop = loop;
    entry->chain = condition->chain = chain;
    condition->frame = plan_storage_frame(lowering->type, statement->condition_owner);
    condition->next = release(lowering, statement, chain, NULL);
    condition->alternative = release(lowering, statement, chain, after);
    if (condition->next == NULL || condition->alternative == NULL) { return NULL; }
    if (!lower_block(lowering, statement->syntax->as.control.body, statement->empty_body ? after : entry,
            after, &loop->body)) { return NULL; }
    PlannedChain *previous = lowering->chain;
    lowering->chain = chain;
    bool queued = lower_operation(lowering, statement->loop_root, after, &entry->next);
    lowering->chain = previous;
    return queued ? entry : NULL;
}
ControlNode *lower_control(Lowering *lowering, const SemanticStatement *statement,
    ControlNode *after, ControlNode *break_target) {
    const AstNode *syntax = statement->syntax;
    bool is_loop = syntax->kind == AST_LOOP;
    PlannedLoop *loop = is_loop ? lower_loop(lowering->type, statement) : NULL;
    if (is_loop && syntax->as.control.command != NULL) { return header(lowering, loop, after, break_target); }
    ControlNode *condition = lower_prepend(lowering, CONTROL_CONDITION, statement, NULL, after);
    if (condition == NULL) { return NULL; }
    condition->alternative = after;
    if (is_loop) {
        loop->entry = loop->repeat = loop->test = condition;
        loop->after = loop->exit = after;
        condition->loop = loop;
        if (!lower_block(lowering, syntax->as.control.body, statement->empty_body ? after : condition,
                after, &loop->body)) { return NULL; }
    } else if (!lower_block(lowering, syntax->as.control.body, after, break_target, &condition->next)
        || !lower_block(lowering, syntax->as.control.alternative, after, break_target, &condition->alternative)) {
        return NULL;
    }
    return condition;
}
