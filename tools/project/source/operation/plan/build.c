/** @file
 * @brief Compose dependency policy, original cuts, prepared image and independently parsed permissions.
 * [TOOL-01](../../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * requires an explicit cross-version mode even with existing permissions;
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires one simultaneous image before final semantic proof.
 */
#include "internal.h"
SourceOperationPlanStatus source_operation_plan_build(SourceOperationPlan *plan, const SourceOperationPlanInputs *inputs) {
    if (!plan || plan->attempted) { return SOURCE_OPERATION_PLAN_INVALID; }
    plan->attempted = true;
    plan->status = source_operation_plan_admit(inputs);
    if (plan->status != SOURCE_OPERATION_PLAN_OK) { return plan->status; }
    plan->inputs = *inputs;
    plan->status = source_operation_plan_removal(plan);
    if (plan->status != SOURCE_OPERATION_PLAN_OK) { return plan->status; }
    if (source_copy_image_prepare(&plan->arena, &inputs->observed->image, inputs->files, inputs->texts,
        inputs->changes, inputs->change_count, inputs->final_roots, inputs->final_root_count, &plan->image, &plan->image_issue) != SOURCE_COPY_IMAGE_OK) { return plan->status = SOURCE_OPERATION_PLAN_IMAGE; }
    if (source_image_capture_build(&plan->capture, plan->image) != SOURCE_IMAGE_CAPTURE_OK) { return plan->status = SOURCE_OPERATION_PLAN_CAPTURE; }
    if (source_package_environment_build(&plan->environment, &plan->capture.catalog) != SOURCE_ENVIRONMENT_OK) { return plan->status = SOURCE_OPERATION_PLAN_ENVIRONMENT; }
    plan->status = source_operation_plan_configurations(plan);
    if (plan->status != SOURCE_OPERATION_PLAN_OK) { return plan->status; }
    plan->mutation_status = source_mutation_forest(&plan->arena, &inputs->observed->image, plan->image, &plan->mutations, &plan->mutation_issue);
    if (plan->mutation_status != SOURCE_FOREST_OK) { plan->failed_path = plan->mutation_issue.path; return plan->status = SOURCE_OPERATION_PLAN_MUTATIONS; }
    plan->prepared = true; return plan->status = SOURCE_OPERATION_PLAN_OK;
}
