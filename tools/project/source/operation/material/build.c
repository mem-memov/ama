/** @file
 * @brief Compose exact request, logical and physical material proofs without changing source state.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires complete immutable preparation before final admission/publication.
 */
#include "internal.h"
SourceOperationMaterialStatus source_operation_material_build(SourceOperationMaterialReview *review,
    const SourceOperationPlan *plan, const SourceOperationIntent *intent) {
    if (!review || review->attempted) { return SOURCE_OPERATION_MATERIAL_INVALID; }
    review->attempted = true;
    if (!plan || !intent) { return review->status = SOURCE_OPERATION_MATERIAL_INVALID; }
    review->intent_status = source_operation_intent_check(plan, intent, &review->intent_issue);
    if (review->intent_status != SOURCE_OPERATION_INTENT_OK) { return review->status = SOURCE_OPERATION_MATERIAL_INTENT; }
    review->plan = plan; review->intent = intent;
    review->explicit_mains.capture = plan->inputs.observed->original;
    review->omissions.copy = plan->inputs.copy; review->omissions.mains = plan->inputs.mains;
    review->status = source_operation_material_explicit(review);
    if (review->status != SOURCE_OPERATION_MATERIAL_OK) { return review->status; }
    review->status = source_operation_material_versions(review);
    if (review->status != SOURCE_OPERATION_MATERIAL_OK) { return review->status; }
    review->status = source_operation_material_logical(review);
    if (review->status != SOURCE_OPERATION_MATERIAL_OK) { return review->status; }
    review->status = source_operation_material_members(review);
    if (review->status != SOURCE_OPERATION_MATERIAL_OK) { return review->status; }
    review->status = source_operation_material_equal(review);
    if (review->status == SOURCE_OPERATION_MATERIAL_OK) { review->valid = true; }
    return review->status;
}
