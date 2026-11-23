/** @file
 * @brief Compose dependency policy, original cuts, prepared image and independently parsed permissions.
 * [TOOL-01](../../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * requires an explicit cross-version mode even with existing permissions;
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires one simultaneous image before final semantic proof.
 */
#include <string.h>
#include "internal.h"
/** Check complete version boundaries, never version-like substrings in names. */
static bool mode(const SourceCopyPlanInputs *inputs) {
    if (inputs->copy->mode != SOURCE_COPY_KEEP_EXTERNAL) { return true; }
    for (const SourceCloneMapping *row = inputs->mains->first; row; row = row->next) {
        const char *old = row->original->logical_path, *now = row->identity;
        ModulePathShape a, b;
        if (!module_path_describe((const unsigned char *)old, strlen(old), &a)
            || !module_path_describe((const unsigned char *)now, strlen(now), &b)) { return false; }
        if (a.version_length != b.version_length || memcmp(old, now, a.version_length)) { return false; }
    }
    return true;
}
SourceCopyPlanStatus source_copy_plan_build(SourceCopyPlan *plan, const SourceCopyPlanInputs *inputs) {
    if (!plan || plan->attempted) { return SOURCE_COPY_PLAN_INVALID; }
    plan->attempted = true;
    if (!inputs || !inputs->copy || !inputs->copy->valid || !inputs->observed || !inputs->observed->valid || !inputs->observed->sealed
        || inputs->observed->original != inputs->copy->tree->capture || !inputs->mains || !inputs->mains->ready
        || inputs->mains->capture != inputs->copy->tree->capture || !inputs->files || !inputs->files->ready
        || !inputs->contexts || !inputs->contexts->ready || inputs->contexts->copy != inputs->copy || inputs->contexts->mains != inputs->mains
        || (inputs->change_count && !inputs->changes) || (inputs->configuration_count && !inputs->configurations)) { return plan->status = SOURCE_COPY_PLAN_INVALID; }
    plan->inputs = *inputs;
    if (!mode(inputs)) { return plan->status = SOURCE_COPY_PLAN_MODE; }
    plan->status = source_copy_plan_removal(plan);
    if (plan->status != SOURCE_COPY_PLAN_OK) { return plan->status; }
    if (source_copy_image_prepare(&plan->arena, &inputs->observed->image, inputs->files, inputs->texts,
        inputs->changes, inputs->change_count, &plan->image, &plan->image_issue) != SOURCE_COPY_IMAGE_OK) { return plan->status = SOURCE_COPY_PLAN_IMAGE; }
    if (source_image_capture_build(&plan->capture, plan->image) != SOURCE_IMAGE_CAPTURE_OK) { return plan->status = SOURCE_COPY_PLAN_CAPTURE; }
    if (source_package_environment_build(&plan->environment, &plan->capture.catalog) != SOURCE_ENVIRONMENT_OK) { return plan->status = SOURCE_COPY_PLAN_ENVIRONMENT; }
    plan->status = source_copy_plan_configurations(plan);
    plan->prepared = plan->status == SOURCE_COPY_PLAN_OK; return plan->status;
}
