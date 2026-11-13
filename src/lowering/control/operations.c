/** @file
 * @brief Queue selected operation subtrees into stable continuation slots without compiler recursion.
 * [CALL-09](../../../spec/language/calls-and-results.md#call-09) is depth first
 * at runtime; finite compiler work follows owned syntax rather than dynamic calls.
 */
#include "internal.h"
bool lower_operation(Lowering *lowering, const SemanticStatement *statement,
    ControlNode *after, ControlNode **destination) {
    LowerTask *task = plan_allocate(lowering->plan, sizeof(*task));
    if (task == NULL) { return false; }
    *task = (LowerTask){.next = lowering->pending, .kind = LOWER_OPERATION_TASK,
        .operation = statement, .chain = lowering->chain, .after = after,
        .break_target = lowering->chain->break_target, .destination = destination};
    lowering->pending = task;
    return true;
}
