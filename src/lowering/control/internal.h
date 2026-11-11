#ifndef AMA_LOWER_CONTROL_INTERNAL_H
#define AMA_LOWER_CONTROL_INTERNAL_H

#include "planning/internal.h"

typedef struct LowerTask {
    struct LowerTask *next;
    AstBlock block;
    ControlNode *after;
    ControlNode *break_target;
    ControlNode **destination;
} LowerTask;
typedef struct {
    ProgramPlan *plan;
    PlannedType *type;
    LowerTask *pending;
} Lowering;

bool lower_block(Lowering *lowering, AstBlock block, ControlNode *after,
    ControlNode *break_target, ControlNode **destination);
ControlNode *lower_statement(Lowering *lowering, const SemanticStatement *statement,
    ControlNode *after, ControlNode *break_target);
ControlNode *lower_call(Lowering *lowering, const SemanticStatement *statement,
    ControlNode *after, ControlNode *break_target);
ControlNode *lower_assignment(Lowering *lowering, const SemanticStatement *statement, ControlNode *after);
ControlNode *lower_prepend(Lowering *lowering, ControlOperation operation,
    const SemanticStatement *statement, const SemanticBinding *binding, ControlNode *after);

#endif
