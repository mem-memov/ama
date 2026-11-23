/** @file
 * @brief Exhaust exact material-review allocations over one genuine immutable copy plan.
 * @see [T276 checkpoint](../../../../../../../docs/implementation/source-operations.md#t276-remaining-implementation-steps-after-t283)
 */
/** Every rederived child prefix must retire without changing the accepted source
 * plan, including failed explicit maps, logical solving and opaque memberships. */
static void check_material_review_failures(const SourceOperationPlan *plan, const SourceOperationIntent *intent) {
    size_t total = 0, baseline = live_allocations;
    for (size_t failure = 0; !failure || failure <= total; failure++) {
        SourceOperationMaterialReview review; source_operation_material_init(&review, sources.allocator);
        size_t before = allocation_count; if (failure) { failure_at = before + failure; }
        SourceOperationMaterialStatus status = source_operation_material_build(&review, plan, intent);
        if (!failure) { assert(status == SOURCE_OPERATION_MATERIAL_OK && review.valid); total = allocation_count - before; assert(total); }
        else { assert(status != SOURCE_OPERATION_MATERIAL_OK && !review.valid); }
        failure_at = 0; source_operation_material_destroy(&review); assert(live_allocations == baseline);
        assert(plan->prepared && plan->inputs.mains->ready && plan->inputs.files->ready);
    }
    SourceOperationMaterialReview review; source_operation_material_init(&review, sources.allocator);
    assert(source_operation_material_build(NULL, plan, intent) == SOURCE_OPERATION_MATERIAL_INVALID);
    assert(source_operation_material_build(&review, NULL, intent) == SOURCE_OPERATION_MATERIAL_INVALID && !review.valid);
    source_operation_material_destroy(&review);
}
