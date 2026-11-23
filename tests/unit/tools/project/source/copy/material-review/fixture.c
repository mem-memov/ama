/** @file
 * @brief Attach genuine raw placement and request intent to the existing cyclic-copy composition.
 * @see [T276 checkpoint](../../../../../../../docs/implementation/source-operations.md#t276-remaining-implementation-steps-after-t283)
 */
/** The fixture copies one selected main, its binary/empty subtree, one dependency
 * and their two associated tests: four mains and exactly ten material entries. */
static void check_copy_material_review(const SourceOperationPlan *plan) {
    const SourceTreeSelection *source = plan->inputs.copy->tree;
    assert(source->count == 1 && S_ISREG(source->operand.metadata.st_mode));
    const SourceCloneMapping *main = plan->inputs.mains->first;
    while (main && main->original != source->units[0]->source) { main = main->next; }
    assert(main);
    SourceTreePlacement placement; source_tree_placement_init(&placement, sources.allocator);
    assert(source_tree_placement_build(&placement, source, main->path) == SOURCE_PLACEMENT_OK);
    SourceOperationIntent intent = {.kind = SOURCE_OPERATION_COPY, .source = source, .placement = &placement};
    SourceOperationMaterialReview review; source_operation_material_init(&review, sources.allocator);
    assert(source_operation_material_build(&review, plan, &intent) == SOURCE_OPERATION_MATERIAL_OK && review.valid);
    assert(review.explicit_mains.first && !review.explicit_mains.first->next && review.destinations.count == 4 && review.version_count == 3);
    size_t count = 0;
    for (const SourceFileMapping *row = review.expected.first; row; row = row->next) { count++; }
    assert(count == 10);
    source_operation_material_destroy(&review);
    check_material_review_failures(plan, &intent);
    check_material_review_missing_test_directory(plan, &intent);
    source_tree_placement_destroy(&placement);
}
