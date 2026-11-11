#ifndef AMA_PLAN_LIFECYCLE_INTERNAL_H
#define AMA_PLAN_LIFECYCLE_INTERNAL_H

#include "planning/internal.h"

LifeNode *plan_life_node(ProgramPlan *plan, PlannedType *type, LifeOperation operation,
    StorageField *field, const AbiBinding *binding);
bool plan_life_append(ProgramPlan *plan, PlannedType *type, LifeProcedure *procedure,
    LifeOperation operation, StorageField *field, const AbiBinding *binding);
bool plan_life_failure(ProgramPlan *plan, PlannedType *type, LifeProcedure *procedure, bool clone);
bool plan_create(ProgramPlan *plan, PlannedType *type, LifeProcedure *procedure, bool literal);
bool plan_destroy(ProgramPlan *plan, PlannedType *type);
bool plan_clone(ProgramPlan *plan, PlannedType *type, LifeProcedure *procedure, bool preserve);

#endif
