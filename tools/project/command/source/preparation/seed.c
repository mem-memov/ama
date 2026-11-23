/** @file
 * @brief Hand actual captured command inputs to the shared seed/closure composition.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * requires original analysis before mapping and forbids a second binding law.
 */
#include "internal.h"
ProjectSourcePreparationStatus project_source_preparation_seed(ProjectSourcePreparation *owner) {
    if (!owner || !owner->captured || !owner->inputs.request || !owner->inputs.request->valid
        || owner->seeds.attempted || (owner->inputs.request->kind != SOURCE_OPERATION_COPY
            && owner->inputs.request->kind != SOURCE_OPERATION_MOVE)) { return PROJECT_SOURCE_PREPARATION_INVALID; }
    if (project_source_seeds_build(&owner->seeds, &owner->placement, &owner->original, owner->inputs.declared,
        owner->inputs.request->dependencies, owner->root_settings.final, owner->root_settings.final_count) != PROJECT_SOURCE_SEEDS_OK) {
        owner->failed_path = owner->seeds.failed_path; return PROJECT_SOURCE_PREPARATION_SEEDS;
    }
    return PROJECT_SOURCE_PREPARATION_OK;
}
