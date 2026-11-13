/**
 * @file
 * @brief Build input/output transfer sequences in exact original binding order.
 *
 * [CALL-08](../../../spec/language/calls-and-results.md#call-08)
 * Reverse work construction preserves forward execution order; literals and omissions have separate installation paths.
 */
#include "internal.h"

ReverseBinding *lower_reverse_bindings(Lowering *lowering, const SemanticStatement *statement) {
    ReverseBinding *reverse = NULL;
    for (const SemanticBinding *binding = statement->bindings; binding != NULL; binding = binding->next) {
        ReverseBinding *frame = plan_allocate(lowering->plan, sizeof(*frame));
        if (frame == NULL) {
            return NULL;
        }
        *frame = (ReverseBinding){reverse, binding};
        reverse = frame;
    }
    return reverse;
}

ControlNode *lower_inputs(Lowering *lowering, const SemanticStatement *statement,
    ReverseBinding *reverse, ControlNode *after) {
    for (; reverse != NULL; reverse = reverse->next) {
        const SemanticBinding *binding = reverse->binding;
        if (binding->syntax->kind == AST_INPUT && !binding->syntax->omitted
            && binding->transfer != TRANSFER_LITERAL) {
            after = lower_prepend(lowering, CONTROL_EVALUATE_INPUT, statement, binding, after);
            if (after == NULL) {
                return NULL;
            }
        }
    }
    return after;
}
