/** @file
 * @brief Check fixed lexical repeats, header release paths and bounded traversal control templates.
 * [FLOW-04](../../../../../spec/language/control-flow.md#flow-04) ends all
 * header grants before body/exit; [FLOW-05](../../../../../spec/language/lists.md#flow-05)
 * keeps empty-body validation distinct from activating a cursor or local cell.
 */
#include "internal.h"
const PlannedLoop *structured_loop(const PlannedType *type, const SemanticStatement *statement) {
    for (const PlannedLoop *loop = type->loops; loop != NULL; loop = loop->next) {
        if (loop->statement == statement) { return loop; }
    }
    return NULL;
}
/** Validate only fixed endpoint relationships, never the body's potentially infinite runtime execution. Component contract: [FLOW-04](../../../../../spec/language/control-flow.md#flow-04), [FLOW-05](../../../../../spec/language/lists.md#flow-05). */
static bool shape(const PlannedType *type, const PlannedLoop *loop) {
    const SemanticStatement *statement = loop->statement;
    if (!structured_node(type, loop->entry) || !structured_node(type, loop->repeat)
        || !structured_node(type, loop->body) || !structured_node(type, loop->exit)
        || !structured_node(type, loop->after)) { return false; }
    if (statement->empty_body && loop->body != loop->after) { return false; }
    if (statement->syntax->kind == AST_FOREACH) {
        if (loop->header != NULL) { return false; }
        if (statement->empty_body) {
            return loop->entry == loop->repeat && loop->test == NULL && loop->exit == loop->after
                && loop->entry->operation == CONTROL_FOREACH_EMPTY && loop->entry->next == loop->after;
        }
        return structured_node(type, loop->test) && loop->entry->operation == CONTROL_FOREACH_ENTER
            && loop->entry->next == loop->test && loop->repeat == loop->test
            && loop->test->operation == CONTROL_FOREACH_NEXT && loop->test->next == loop->body
            && loop->test->alternative == loop->exit && loop->exit->operation == CONTROL_FOREACH_LEAVE
            && loop->exit->next == loop->after;
    }
    if (!structured_node(type, loop->test) || loop->test->operation != CONTROL_CONDITION || loop->exit != loop->after) { return false; }
    if (statement->syntax->as.control.command == NULL) {
        return loop->header == NULL && loop->entry == loop->test && loop->repeat == loop->test
            && loop->test->next == loop->body && loop->test->alternative == loop->after;
    }
    const ControlNode *yes = loop->test->next;
    const ControlNode *no = loop->test->alternative;
    return structured_chain(type, loop->header) && loop->header->header == loop
        && loop->entry == loop->repeat && loop->entry->operation == CONTROL_HEADER_ENTER
        && yes->operation == CONTROL_RELEASE_CHAIN && no->operation == CONTROL_RELEASE_CHAIN
        && yes->chain == loop->header && no->chain == loop->header
        && yes->next == loop->body && no->next == loop->after;
}
bool structured_loops(const PlannedType *type) {
    const PlannedLoop *loop = type->loops;
    const PlannedLoop *last = NULL;
    bool traversals = false;
    for (const SemanticStatement *statement = type->type->statements; statement != NULL; statement = statement->next) {
        if (statement->syntax->kind != AST_LOOP && statement->syntax->kind != AST_FOREACH) { continue; }
        traversals |= statement->syntax->kind == AST_FOREACH;
        if (loop == NULL || loop->statement != statement || loop->parent != structured_loop(type, statement->enclosing_loop)
            || loop->frame != plan_storage_frame(type, statement) || !shape(type, loop)) { return false; }
        last = loop;
        loop = loop->next;
    }
    if (loop != NULL || last != type->last_loop) { return false; }
    if (!traversals) { return type->resume_entry == NULL; }
    return type->entry->operation == CONTROL_RESET_SCOPES && structured_node(type, type->resume_entry)
        && type->resume_entry->operation == CONTROL_VALIDATE_TRAVERSALS
        && structured_node(type, type->resume_entry->next)
        && type->resume_entry->next->operation == CONTROL_RESUME_SAVED && type->resume_entry->next->next == NULL;
}
