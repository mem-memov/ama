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

/** Plan-owned reverse binding view for exact forward input/output text order under prepend construction. Component contract: [CALL-08](../../../spec/language/calls-and-results.md#call-08). */
typedef struct ReverseBinding {
    struct ReverseBinding *next;
    const SemanticBinding *binding;
} ReverseBinding;
/** Build plan-owned reversal work so prepending preserves exact text order; null may mean empty or failure, distinguished by plan status.
 * Component contract: [CALL-08](../../../spec/language/calls-and-results.md#call-08). */
ReverseBinding *lower_reverse_bindings(Lowering *lowering, const SemanticStatement *statement);
/** Lower all typed output actions, queueing selected children without recursive compiler calls. Component contract: [CALL-08](../../../spec/language/calls-and-results.md#call-08). */
ControlNode *lower_actions(Lowering *, const SemanticStatement *, ControlNode *);
/** Build whole-chain abandonment before this exact link's handler, preserving the root's caller loop context. Component contract: [CALL-08](../../../spec/language/calls-and-results.md#call-08). */
ControlNode *lower_refusal(Lowering *, const SemanticStatement *);
/** Lower one intrinsic method's staged inputs/preflight/commit with no synthetic module invocation. Component contract: [CALL-08](../../../spec/language/calls-and-results.md#call-08). */
ControlNode *lower_list_method(Lowering *, const SemanticStatement *, ControlNode *, ControlNode *);
/** Prepend only supplied nonliteral source evaluations in original input order; literal construction belongs to installation.
 * Component contract: [CALL-08](../../../spec/language/calls-and-results.md#call-08). */
ControlNode *lower_inputs(Lowering *lowering, const SemanticStatement *statement,
    ReverseBinding *reverse, ControlNode *after);
/** Walk callee fields backward while prepending, yielding forward parameter/result declaration order and excluding persistent state.
 * Component contract: [CALL-08](../../../spec/language/calls-and-results.md#call-08). */
ControlNode *lower_install(Lowering *lowering, const SemanticStatement *statement,
    ControlNode *after);

#endif
