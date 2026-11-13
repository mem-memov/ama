/** @file
 * @brief Discard only the initiating chain before the refused link's own caller-side handler.
 * [CALL-09](../../../../spec/language/calls-and-results.md#call-09) abandons all
 * unfinished siblings and ancestors; handlers retain the incoming loop context.
 */
#include "lowering/calls/internal.h"
ControlNode *lower_refusal(Lowering *lowering, const SemanticStatement *statement) {
    PlannedChain *chain = lowering->chain;
    ControlNode *after = lowering->type->failure;
    if (statement->syntax->as.call.has_alternative) {
        after = lower_prepend(lowering, CONTROL_REFUSAL_SNAPSHOT, statement, NULL, chain->exit);
        if (after == NULL || !lower_block(lowering, statement->syntax->as.call.alternative,
                chain->exit, chain->break_target, &after->next)) { return NULL; }
    }
    if (chain->header != NULL || chain->root->syntax->as.call.has_selected) {
        after = lower_prepend(lowering, CONTROL_ABANDON_CHAIN, statement, NULL, after);
        if (after == NULL) { return NULL; }
        after->chain = chain;
    }
    return after;
}
