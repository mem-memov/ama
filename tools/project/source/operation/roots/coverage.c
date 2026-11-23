/** @file
 * @brief Decide original-analysis coverage by exact physical subtree/exclusion boundaries.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * requires newly covered sources in the original analysis; observed physical
 * ancestors alone do not provide semantic coverage under
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04).
 */
#include "internal.h"
SourceOperationRootsStatus source_operation_roots_coverage(SourceOperationRoots *owner) {
    const SourceOperationRootsInputs *inputs = &owner->inputs;
    const SourceRootCapture *provided = inputs->plan->inputs.observed->original;
    for (size_t i = 0; i < inputs->configured->root_count; i++) {
        SourceOperationRootsStatus status = source_operation_root_region(&owner->arena, &inputs->configured->roots[i],
            provided->roots, provided->root_count, &owner->failed_path);
        if (status != SOURCE_OPERATION_ROOTS_OK) { return status; }
    }
    for (size_t i = 0; i < inputs->plan->inputs.final_root_count; i++) {
        const ModuleRoot *root = &inputs->plan->inputs.final_roots[i];
        if (source_image_path_absent(&inputs->plan->inputs.observed->image, root->path)) { continue; }
        SourceOperationRootsStatus status = source_operation_root_region(&owner->arena, root, provided->roots,
            provided->root_count, &owner->failed_path);
        if (status != SOURCE_OPERATION_ROOTS_OK) { return status; }
    }
    owner->failed_path = NULL; return SOURCE_OPERATION_ROOTS_OK;
}
