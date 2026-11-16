/**
 * @file
 * @brief Declare typed lifecycle-node and procedure construction helpers.
 *
 * [LIFE-01](../../../spec/implementation/native-interface.md#life-01)
 * Every partial construction failure has an explicit unwind path and one owning plan arena.
 */
#ifndef AMA_PLAN_LIFECYCLE_INTERNAL_H
#define AMA_PLAN_LIFECYCLE_INTERNAL_H

#include "planning/internal.h"

/** Allocate one typed lifecycle node with exact field/native hook and borrowed source site.
 * Component contract: [LIFE-01](../../../spec/implementation/native-interface.md#life-01). */
LifeNode *plan_life_node(ProgramPlan *plan, PlannedType *type, LifeOperation operation,
    StorageField *field, const AbiBinding *binding);
/** Append one lifecycle step and attach its owning procedure failure edge; failure retains only plan-owned scratch.
 * Component contract: [LIFE-01](../../../spec/implementation/native-interface.md#life-01). */
bool plan_life_append(ProgramPlan *plan, PlannedType *type, LifeProcedure *procedure,
    LifeOperation operation, StorageField *field, const AbiBinding *binding);
/** Build initialized-prefix unwind and optional map-release failure path before normal construction steps.
 * Component contract: [LIFE-01](../../../spec/implementation/native-interface.md#life-01). */
bool plan_life_failure(ProgramPlan *plan, PlannedType *type, LifeProcedure *procedure, bool clone);
/** Plan declared fields then one selected default/literal native payload, live marker and success.
 * Component contract: [LIFE-01](../../../spec/implementation/native-interface.md#life-01). */
bool plan_create(ProgramPlan *plan, PlannedType *type, LifeProcedure *procedure, bool literal);
/** Plan native payload first, reverse fields then finalized marker; accumulate cleanup errors rather than stop after the first.
 * Component contract: [LIFE-01](../../../spec/implementation/native-interface.md#life-01). */
bool plan_destroy(ProgramPlan *plan, PlannedType *type);
/** Plan whole-boundary preflight/map/owned-reference-native copy/fixup and explicit preserve-or-clear control state.
 * Component contract: [LIFE-01](../../../spec/implementation/native-interface.md#life-01). */
bool plan_clone(ProgramPlan *plan, PlannedType *type, LifeProcedure *procedure, bool preserve);

#endif
