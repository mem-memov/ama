/** @file
 * @brief Rebuild complete raw, structural, associated-test and declared membership with shared operations.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * requires these complete sets and their exact agreement at every overlap.
 */
#include "internal.h"
SourceOperationMaterialStatus source_operation_material_members(SourceOperationMaterialReview *review) {
    const SourceCopyClosure *copy = review->plan->inputs.copy;
    if (copy) {
        if (source_file_map_tree(&review->expected, review->intent->placement) != SOURCE_FILE_MAP_OK) { return SOURCE_OPERATION_MATERIAL_RAW; }
        review->units_status = source_copy_map_unit_members(&review->expected, copy,
            review->plan->inputs.mains, &review->omissions, &review->units_issue);
        if (review->units_status != SOURCE_COPY_UNITS_OK) { return SOURCE_OPERATION_MATERIAL_UNITS; }
        if (source_copy_context_omissions_seal(&review->omissions) != SOURCE_COPY_CONTEXT_OK) { return SOURCE_OPERATION_MATERIAL_UNITS; }
        review->tests_status = source_copy_map_test_members(&review->expected, copy,
            review->plan->inputs.mains, review->versions, review->version_count, &review->tests_issue);
        if (review->tests_status != SOURCE_COPY_TESTS_OK) { return SOURCE_OPERATION_MATERIAL_TESTS; }
        review->declared_status = source_copy_map_declared_members(&review->expected, &review->destinations,
            review->versions, review->version_count, &review->declared_issue);
        if (review->declared_status != SOURCE_COPY_DECLARED_OK) { return SOURCE_OPERATION_MATERIAL_DECLARED; }
    }
    return source_file_map_seal(&review->expected) == SOURCE_FILE_MAP_OK ? SOURCE_OPERATION_MATERIAL_OK : SOURCE_OPERATION_MATERIAL_FILESET;
}
