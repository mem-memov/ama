/** @file
 * @brief Compose one closed source-operation construction path without command-specific planners.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires every write, cut and setting before publication; independently checked
 * final compiler facts arrive only at the subsequent single-use sealing boundary.
 */
#include "internal.h"
#include "../plan/internal.h"
SourceOperationCompleteStatus source_operation_complete_prepare(SourceOperationComplete *owner, const SourceOperationCompleteInputs *inputs) {
    if (!owner || owner->attempted) { return SOURCE_OPERATION_COMPLETE_INVALID; }
    owner->attempted = true; owner->status = SOURCE_OPERATION_COMPLETE_INVALID;
    if (!inputs || !inputs->writes || !inputs->intent || !inputs->original || !inputs->original->valid
        || !inputs->configured || !inputs->configured->valid || (inputs->final_root_count && !inputs->final_roots)) { return owner->status; }
    owner->inputs = *inputs;
    if (source_operation_writes_build(&owner->writes, inputs->writes) != SOURCE_OPERATION_WRITES_OK) {
        return owner->status = SOURCE_OPERATION_COMPLETE_WRITES;
    }
    const SourceOperationWritesInputs *writes = &owner->writes.inputs;
    if (source_package_environment_build(&owner->original_environment, &writes->observed->original->catalog) != SOURCE_ENVIRONMENT_OK) {
        return owner->status = SOURCE_OPERATION_COMPLETE_ENVIRONMENT;
    }
    SourceOperationPlanInputs plan = {.original = inputs->original, .final_roots = inputs->final_roots,
        .final_root_count = inputs->final_root_count, .copy = writes->copy, .observed = writes->observed,
        .mains = writes->mains, .files = writes->files, .texts = owner->writes.prepared,
        .contexts = inputs->contexts, .changes = owner->writes.changes, .change_count = owner->writes.count,
        .removal = writes->removal, .inputs = inputs->inputs, .clients = inputs->clients};
    if (source_operation_plan_project(&owner->plan, &plan) != SOURCE_OPERATION_PLAN_OK) { return owner->status = SOURCE_OPERATION_COMPLETE_PLAN; }
    owner->status = source_operation_complete_companions(owner);
    if (owner->status != SOURCE_OPERATION_COMPLETE_OK) { return owner->status; }
    owner->status = source_operation_complete_configurations(owner);
    if (owner->status != SOURCE_OPERATION_COMPLETE_OK) { return owner->status; }
    if (source_operation_plan_finish(&owner->plan, owner->configurations, owner->configuration_count) != SOURCE_OPERATION_PLAN_OK) {
        return owner->status = SOURCE_OPERATION_COMPLETE_PLAN;
    }
    if (source_operation_material_build(&owner->material, &owner->plan, inputs->intent) != SOURCE_OPERATION_MATERIAL_OK) {
        return owner->status = SOURCE_OPERATION_COMPLETE_MATERIAL;
    }
    SourceOperationRootsInputs roots = {.configured = inputs->configured, .plan = &owner->plan,
        .originals = inputs->root_originals, .original_count = inputs->root_original_count,
        .additions = inputs->root_additions, .addition_count = inputs->root_addition_count};
    if (source_operation_roots_build(&owner->roots, &roots) != SOURCE_OPERATION_ROOTS_OK) { return owner->status = SOURCE_OPERATION_COMPLETE_ROOTS; }
    owner->prepared = true; return owner->status = SOURCE_OPERATION_COMPLETE_OK;
}
