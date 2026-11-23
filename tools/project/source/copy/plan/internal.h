/** @file
 * @brief Keep original removal and final configuration obligations separate from image orchestration.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * composes distinct evidence without replacing original-cut checks with final typing.
 */
#ifndef AMA_SOURCE_COPY_PLAN_INTERNAL_H
#define AMA_SOURCE_COPY_PLAN_INTERNAL_H
#include "plan.h"
/** Check exact original physical removal material and combined consumers. */
SourceCopyPlanStatus source_copy_plan_removal(SourceCopyPlan *);
/** Account for every independently rebuilt actual final package exactly once. */
SourceCopyPlanStatus source_copy_plan_configurations(SourceCopyPlan *);
#endif
