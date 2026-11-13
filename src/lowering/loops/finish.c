/** @file
 * @brief Patch loop body entries and make traversal restart/resume policies explicit before publication.
 * [CONT-02](../../../spec/language/continuations.md#cont-02) saves body positions
 * independently of C activations; [FLOW-05](../../../spec/language/lists.md#flow-05)
 * validates active cursors outermost first and discards exited scopes without destroying referents.
 */
#include "lowering/control/internal.h"
bool lower_finish_loops(Lowering *lowering) {
    bool traversals = false;
    for (PlannedLoop *loop = lowering->type->loops; loop != NULL; loop = loop->next) {
        bool traversal = loop->statement->syntax->kind == AST_FOREACH;
        traversals |= traversal;
        if (traversal && loop->statement->empty_body) { continue; }
        if (loop->header != NULL) { loop->test->next->next = loop->body; }
        else { loop->test->next = loop->body; }
    }
    if (!traversals) { return true; }
    ControlNode *reset = lower_prepend(lowering, CONTROL_RESET_SCOPES, NULL, NULL, lowering->type->entry);
    ControlNode *resume = reset == NULL ? NULL : lower_prepend(lowering, CONTROL_RESUME_SAVED, NULL, NULL, NULL);
    ControlNode *validate = resume == NULL ? NULL : lower_prepend(lowering, CONTROL_VALIDATE_TRAVERSALS, NULL, NULL, resume);
    if (validate == NULL) { return false; }
    lowering->type->entry = reset;
    lowering->type->resume_entry = validate;
    return true;
}
