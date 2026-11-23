/** @file
 * @brief Keep original removal and final configuration obligations separate from image orchestration.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * composes distinct evidence without replacing original-cut checks with final typing.
 */
#ifndef AMA_SOURCE_OPERATION_PLAN_INTERNAL_H
#define AMA_SOURCE_OPERATION_PLAN_INTERNAL_H
#include "plan.h"
/** Admit original-input equality and disjoint copy/deletion provenance/modes. */
SourceOperationPlanStatus source_operation_plan_admit(const SourceOperationPlanInputs *);
/** Check exact original physical removal material and combined consumers. */
SourceOperationPlanStatus source_operation_plan_removal(SourceOperationPlan *);
/** Account for every independently rebuilt actual final package exactly once. */
SourceOperationPlanStatus source_operation_plan_configurations(SourceOperationPlan *);
#endif
