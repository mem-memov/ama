/** @file
 * @brief Recompute additional active ordinary destinations from genuine raw C seeds.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * preserves each additional unit's specified suffix and test association instead
 * of accepting an arbitrary compilable rename.
 */
#include "internal.h"
#include <string.h>
SourceOperationMaterialStatus source_operation_material_logical(SourceOperationMaterialReview *review) {
    const SourceCopyClosure *copy = review->plan->inputs.copy;
    if (!copy) { return SOURCE_OPERATION_MATERIAL_OK; }
    if (source_copy_destinations_build(&review->destinations, copy, &review->explicit_mains) != SOURCE_COPY_DESTINATIONS_OK) {
        return SOURCE_OPERATION_MATERIAL_DESTINATIONS;
    }
    for (size_t i = 0; i < review->destinations.count; i++) {
        const SourceCopyDestination *expected = &review->destinations.entries[i];
        const SourceCloneMapping *actual = review->plan->inputs.mains->first;
        while (actual && actual->original != expected->source) { actual = actual->next; }
        if (!actual || strcmp(actual->identity, expected->identity)) {
            review->failed_main = expected->source; review->failed_path = expected->identity;
            return SOURCE_OPERATION_MATERIAL_LOGICAL;
        }
    }
    return SOURCE_OPERATION_MATERIAL_OK;
}
