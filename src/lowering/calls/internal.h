/**
 * @file
 * @brief Declare reverse-binding and declaration-order call preparation helpers.
 *
 * [CALL-08](../../../spec/language/calls-and-results.md#call-08)
 * Input/output text order and callee installation order are deliberately distinct resolved sequences.
 */
#ifndef AMA_LOWER_CALLS_INTERNAL_H
#define AMA_LOWER_CALLS_INTERNAL_H

#include "lowering/control/internal.h"

/** Plan-owned reverse binding view for exact forward input/output text order under prepend construction. */
typedef struct ReverseBinding {
    struct ReverseBinding *next;
    const SemanticBinding *binding;
} ReverseBinding;
/** Build plan-owned reversal work so prepending preserves exact text order; null may mean empty or failure, distinguished by plan status.
 */
ReverseBinding *lower_reverse_bindings(Lowering *lowering, const SemanticStatement *statement);
/** Prepend only nonomitted output transfers in original binding order; retain exact receiver metadata for later evaluation.
 */
ControlNode *lower_outputs(Lowering *lowering, const SemanticStatement *statement,
    ReverseBinding *reverse, ControlNode *after);
/** Prepend only supplied nonliteral source evaluations in original input order; literal construction belongs to installation.
 */
ControlNode *lower_inputs(Lowering *lowering, const SemanticStatement *statement,
    ReverseBinding *reverse, ControlNode *after);
/** Walk callee fields backward while prepending, yielding forward parameter/result declaration order and excluding persistent state.
 */
ControlNode *lower_install(Lowering *lowering, const SemanticStatement *statement,
    ControlNode *after);

#endif
