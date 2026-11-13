/** @file
 * @brief Lower foreach to fixed collection-entry, live-boundary selection and nonowning scope cleanup.
 * [FLOW-05](../../../spec/language/lists.md#flow-05) activates cells only for
 * eligible entries and leaves no acquisition across body/yield. Empty parsed
 * bodies validate their collection once without activating any cursor or cell.
 */
#include "lowering/control/internal.h"
ControlNode *lower_foreach(Lowering *lowering, const SemanticStatement *statement, ControlNode *after) {
    PlannedLoop *loop = lower_loop(lowering->type, statement);
    loop->after = after;
    if (statement->empty_body) {
        ControlNode *empty = lower_prepend(lowering, CONTROL_FOREACH_EMPTY, statement, NULL, after);
        if (empty == NULL) { return NULL; }
        empty->loop = loop;
        loop->entry = loop->repeat = empty;
        loop->body = loop->exit = after;
        return empty;
    }
    ControlNode *leave = lower_prepend(lowering, CONTROL_FOREACH_LEAVE, statement, NULL, after);
    ControlNode *next = leave == NULL ? NULL : lower_prepend(lowering, CONTROL_FOREACH_NEXT, statement, NULL, NULL);
    ControlNode *enter = next == NULL ? NULL : lower_prepend(lowering, CONTROL_FOREACH_ENTER, statement, NULL, next);
    if (enter == NULL) { return NULL; }
    enter->loop = next->loop = leave->loop = loop;
    next->alternative = leave;
    loop->entry = enter;
    loop->repeat = loop->test = next;
    loop->exit = leave;
    return lower_block(lowering, statement->syntax->as.traversal.body, next, leave, &loop->body) ? enter : NULL;
}
