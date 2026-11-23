/** @file
 * @brief Compose dependency policy, original cuts, prepared image and independently parsed permissions.
 * [TOOL-01](../../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * requires an explicit cross-version mode even with existing permissions;
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires one simultaneous image before final semantic proof.
 */
#include "internal.h"
SourceOperationPlanStatus source_operation_plan_build(SourceOperationPlan *plan, const SourceOperationPlanInputs *inputs) {
    SourceOperationPlanStatus status = source_operation_plan_project(plan, inputs);
    if (status != SOURCE_OPERATION_PLAN_OK) { return status; }
    return source_operation_plan_finish(plan, inputs->configurations, inputs->configuration_count);
}
