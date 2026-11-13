/** @file
 * @brief Lower ordered bindings and selected subtrees to explicit depth-first continuations.
 * [CALL-09](../../../../spec/language/calls-and-results.md#call-09) completes a
 * selected subtree before later siblings and keeps ordinary operands in the
 * initiating caller. Work records, not compiler recursion, patch child entries.
 */
#include "lowering/calls/internal.h"
#include "lowering/replacement/internal.h"
/** Plan-owned reverse action view preserves source order under backward composition. */
typedef struct ReverseAction {
    struct ReverseAction *next;
    const SemanticOutputAction *action;
} ReverseAction;
ControlNode *lower_actions(Lowering *lowering, const SemanticStatement *statement,
    ControlNode *after) {
    ReverseAction *reverse = NULL;
    for (const SemanticOutputAction *action = statement->actions; action != NULL; action = action->next) {
        ReverseAction *item = plan_allocate(lowering->plan, sizeof(*item));
        if (item == NULL) { return NULL; }
        *item = (ReverseAction){reverse, action};
        reverse = item;
    }
    for (; reverse != NULL; reverse = reverse->next) {
        const SemanticOutputAction *action = reverse->action;
        if (action->binding != NULL) {
            if (action->binding->syntax->omitted) { continue; }
            after = lower_replacement(lowering, statement, action->binding, NULL, REPLACE_OUTPUT, after);
            if (after == NULL) { return NULL; }
        } else {
            ControlNode *entry = lower_prepend(lowering, CONTROL_ACTION_NEXT, action->operation, NULL, NULL);
            if (entry == NULL) { return NULL; }
            entry->chain = lowering->chain;
            entry->resume = after;
            if (!lower_operation(lowering, action->operation, after, &entry->next)) { return NULL; }
            after = entry;
        }
    }
    return after;
}
