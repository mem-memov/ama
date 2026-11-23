/** @file
 * @brief Produce the single simultaneous image/environment before deriving final configuration expectations.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires source, configuration and removal facts from the same final image;
 * this private phase deliberately does not certify a prepared plan.
 */
#include "internal.h"
SourceOperationPlanStatus source_operation_plan_project(SourceOperationPlan *plan, const SourceOperationPlanInputs *inputs) {
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
    plan->projected = true; return plan->status = SOURCE_OPERATION_PLAN_OK;
}
