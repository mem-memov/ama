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

/** Finite queued work distinguishes ordinary bodies from selected operation subtrees. */
typedef enum { LOWER_BLOCK_TASK, LOWER_OPERATION_TASK } LowerTaskKind;
/** Plan-owned pending block lowering with stable borrowed entry slot and exact continuation/break target. */
typedef struct LowerTask {
    struct LowerTask *next;
    LowerTaskKind kind;
    const SemanticStatement *operation;
    PlannedChain *chain;
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
    PlannedChain *chain;
} Lowering;

/** Queue a finite borrowed block with exact after/break continuations and a stable plan-owned entry slot; do not recursively lower it.
 */
bool lower_block(Lowering *lowering, AstBlock block, ControlNode *after,
    ControlNode *break_target, ControlNode **destination);
/** Queue a selected operation, keeping its initiating chain and a stable caller-owned entry slot. */
bool lower_operation(Lowering *, const SemanticStatement *, ControlNode *, ControlNode **);
/** Allocate and link source loop records before any body/control labels are constructed. */
bool lower_loop_inventory(Lowering *);
/** Patch stable loop body slots and add fresh-entry/resume traversal cleanup/validation templates. */
bool lower_finish_loops(Lowering *);
/** Resolve the exact source-loop identity in this planned type. */
PlannedLoop *lower_loop(const PlannedType *, const SemanticStatement *);
/** Lower a named/header loop or branch with fixed lexical repeat/exit continuations. */
ControlNode *lower_control(Lowering *, const SemanticStatement *, ControlNode *, ControlNode *);
/** Lower one foreach using its fixed cursor/cell frame and scope cleanup continuations. */
ControlNode *lower_foreach(Lowering *, const SemanticStatement *, ControlNode *);
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
