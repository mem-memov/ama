#ifndef AMA_TYPED_PLAN_INTERNAL_H
#define AMA_TYPED_PLAN_INTERNAL_H

#include "plan.h"

void *plan_allocate(ProgramPlan *plan, size_t size);
bool plan_fail(ProgramPlan *plan, SourceSpan site, const char *message);
bool plan_storage(ProgramPlan *plan);
bool plan_lifecycles(ProgramPlan *plan, PlannedType *type);
bool plan_control(ProgramPlan *plan, PlannedType *type);
ControlNode *plan_control_node(ProgramPlan *plan, PlannedType *type,
    ControlOperation operation, const SemanticStatement *statement);
const SemanticStatement *plan_statement(const PlannedType *type, const AstNode *syntax);

#endif
