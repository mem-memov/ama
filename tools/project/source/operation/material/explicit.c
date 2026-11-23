/** @file
 * @brief Recover exact raw selected-main proposals from the prepared injective C relation.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * derives additional destinations from actual explicit inputs, never guessed names.
 */
#include "internal.h"
SourceOperationMaterialStatus source_operation_material_explicit(SourceOperationMaterialReview *review) {
    if (review->plan->inputs.copy) {
        const SourceTreeSelection *tree = review->intent->source;
        for (size_t i = 0; i < tree->count; i++) {
            const ModuleSource *source = tree->units[i]->source;
            const SourceCloneMapping *row = review->plan->inputs.mains->first;
            while (row && row->original != source) { row = row->next; }
            review->failed_main = source; review->failed_path = source_path(source->text);
            if (!row) { return SOURCE_OPERATION_MATERIAL_EXPLICIT; }
            const SourceCloneMapping *added = NULL;
            if (source_clone_map_add(&review->explicit_mains, source, row->path, row->identity, &added) != SOURCE_CLONE_OK) {
                return SOURCE_OPERATION_MATERIAL_EXPLICIT;
            }
        }
    }
    review->failed_main = NULL; review->failed_path = NULL;
    return source_clone_map_seal(&review->explicit_mains) == SOURCE_CLONE_OK ? SOURCE_OPERATION_MATERIAL_OK : SOURCE_OPERATION_MATERIAL_EXPLICIT;
}
