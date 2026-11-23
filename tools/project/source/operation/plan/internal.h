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
/** Private first phase: admit the exact original/cut and project one image,
 * capture and environment. Mark only projected; this grants no prepared-plan
 * authority. The complete owner can derive expectations from actual final scope
 * facts without creating a second projection or fabricating a package scope. */
SourceOperationPlanStatus source_operation_plan_project(SourceOperationPlan *, const SourceOperationPlanInputs *);
/** Private single-use completion: admit supplied exact configuration expectations
 * and derive the mutation forest. Borrow rows through plan destruction. Mark
 * prepared only after both gates succeed; refuse unprojected/finished owners. */
SourceOperationPlanStatus source_operation_plan_finish(SourceOperationPlan *,
    const SourceOperationConfigurationExpectation *, size_t);
#endif
