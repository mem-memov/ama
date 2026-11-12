/**
 * @file
 * @brief Declare explicit reverse-block lowering tasks and typed continuation slots.
 *
 * [GEN-02](../../../spec/implementation/compiler.md#gen-02)
 * Represent pending nested control with arena records, not recursive compiler C activations.
 */
#ifndef AMA_LOWER_CONTROL_INTERNAL_H
#define AMA_LOWER_CONTROL_INTERNAL_H

#include "planning/internal.h"

/** Plan-owned pending block lowering with stable borrowed entry slot and exact continuation/break target. */
typedef struct LowerTask {
    struct LowerTask *next;
    AstBlock block;
    ControlNode *after;
    ControlNode *break_target;
    ControlNode **destination;
} LowerTask;
/** Narrow lowering view over its owning plan, one planned type and pending explicit work. */
typedef struct {
    ProgramPlan *plan;
    PlannedType *type;
    LowerTask *pending;
} Lowering;

/** Queue a finite borrowed block with exact after/break continuations and a stable plan-owned entry slot; do not recursively lower it.
 */
bool lower_block(Lowering *lowering, AstBlock block, ControlNode *after,
    ControlNode *break_target, ControlNode **destination);
/** Lower one already resolved retained AST statement; no declaration lookup, runtime evaluation or source-type inference.
 */
ControlNode *lower_statement(Lowering *lowering, const SemanticStatement *statement,
    ControlNode *after, ControlNode *break_target);
/** Compose resolved acquisition/input/return/output graph with a lexical refusal continuation; constants bypass executable acquisition.
 */
ControlNode *lower_call(Lowering *lowering, const SemanticStatement *statement,
    ControlNode *after, ControlNode *break_target);
/** Retain every receiver/source/replacement/release step regardless of source spelling. Runtime identity handles ordinary no-op after checks.
 */
ControlNode *lower_assignment(Lowering *lowering, const SemanticStatement *statement, ControlNode *after);
/** Allocate a typed node preceding a known continuation, borrowing exact statement/binding identities and original sites.
 */
ControlNode *lower_prepend(Lowering *lowering, ControlOperation operation,
    const SemanticStatement *statement, const SemanticBinding *binding, ControlNode *after);

#endif
