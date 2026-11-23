/** @file
 * @brief Exercise joint move/replacement cuts through actual compiler graphs and final images.
 * @see [T224 contract](../../../../../../../docs/implementation/source-operations.md)
 */
/** Rebuild a frozen original after constructing a mutual-reference fixture;
 * no production source operation is allowed to change its admitted snapshot. */
static void merge_fixture(void) {
    begin_removal(MATERIAL_NONE, main_path);
    source_removal_selection_destroy(&removal); source_analysis_destroy(&analysis);
    source_tree_selection_destroy(&selection); source_root_capture_destroy(&capture);
    file("generated/project/packages/vendor/pkg/v1.0.0/src/parent+/child", "@!\nother&\n/vendor/pkg/v1.0.0/src/parentish\n");
    file("generated/project/packages/vendor/pkg/v1.0.0/src/parentish", "@!\nother&\n/vendor/pkg/v1.0.0/src/parent/child\n");
    selection_capture_scope(false, false);
    source_analysis_init(&analysis, sources.allocator);
    FILE *request = fopen("generated/request.json", "wb"); assert(request);
    assert(delivery_source_snapshot_write(request, &capture.view) && fclose(request) == 0);
    assert(source_analysis_check(&analysis, &capture.view, "../../../../../../ama", "generated/request.json") == SOURCE_ANALYSIS_OK);
    assert(source_tree_selection_read(&selection, &capture, main_path) == SOURCE_TREE_OK);
    source_removal_selection_init(&removal, sources.allocator);
    assert(source_removal_selection_build(&removal, &selection, &analysis, NULL) == SOURCE_REMOVAL_SELECTION_OK);
}
/** Individually blocked removals become safe exactly when both original owners
 * disappear; explicit outside client roots still block the combined operation. */
static void it_merges_joint_original_cuts_and_proves_the_actual_final_image(void) {
    merge_fixture();
    SourceTreeSelection tree; source_tree_selection_init(&tree, sources.allocator);
    assert(source_tree_selection_read(&tree, &capture, "generated/project/packages/vendor/pkg/v1.0.0/src/parentish") == SOURCE_TREE_OK);
    SourceRemovalSelection second, merged;
    source_removal_selection_init(&second, sources.allocator); source_removal_selection_init(&merged, sources.allocator);
    assert(source_removal_selection_build(&second, &tree, &analysis, NULL) == SOURCE_REMOVAL_SELECTION_OK);
    SourceRemovalReview review;
    assert(source_removal_selection_review(&removal.arena, &removal, NULL, NULL, &review) == SOURCE_REMOVAL_BLOCKED);
    assert(source_removal_selection_review(&second.arena, &second, NULL, NULL, &review) == SOURCE_REMOVAL_BLOCKED);
    const SourceRemovalSelection *parts[] = {&removal, &second, &removal};
    assert(source_removal_selection_merge(&merged, parts, 3) == SOURCE_REMOVAL_SELECTION_OK);
    assert(merged.path_count == removal.path_count + second.path_count && merged.identity_count == 3);
    size_t expected = 0, actual = 0;
    for (const SourceRemovalInputUse *use = removal.inputs; use; use = use->next) { expected++; }
    for (const SourceRemovalInputUse *use = merged.inputs; use; use = use->next) { actual++; }
    assert(actual == expected && actual);
    assert(source_removal_selection_review(&merged.arena, &merged, NULL, NULL, &review) == SOURCE_REMOVAL_OK);
    SourceClientRoot root = {.definition = definition("/vendor/pkg/v1.0.0/src/parentish")};
    assert(source_removal_selection_review(&merged.arena, &merged, NULL, &root, &review) == SOURCE_REMOVAL_BLOCKED && review.clients);
    RemovalPlanFixture fixture; assert(removal_plan_fixture_build(&fixture, &merged));
    SourceOperationPlan *image = &fixture.plan;
    FILE *request = fopen("generated/final-merged.json", "wb"); assert(request);
    assert(delivery_source_snapshot_write(request, &image->capture.view) && fclose(request) == 0);
    SourceCompilerClient final; source_compiler_client_init(&final, sources.allocator);
    assert(source_compiler_client_run(&final, "../../../../../../ama", "generated/final-merged.json", &image->capture.view, NULL) == SOURCE_COMPILER_OK);
    SourceCopyProofIssue issue;
    assert(source_operation_plan_prove(image, &final, &issue) == SOURCE_COPY_PROOF_OK);
    source_compiler_client_destroy(&final); removal_plan_fixture_destroy(&fixture);
    source_removal_selection_destroy(&merged); source_removal_selection_destroy(&second);
    source_tree_selection_destroy(&tree); end_removal(MATERIAL_NONE);
}
/** Exact owner admission, every allocation ordinal, overlap and contradictory
 * repeated observations must never publish an accepted partial removal union. */
static void it_retires_merged_removal_failures_and_rejects_foreign_owners(void) {
    begin_removal(MATERIAL_NONE, main_path);
    assert(source_removal_selection_build(&removal, &selection, &analysis, NULL) == SOURCE_REMOVAL_SELECTION_OK);
    const SourceRemovalSelection *parts[] = {&removal, &removal}; size_t count = 0;
    for (size_t failure = 0; !failure || failure <= count; failure++) {
        SourceRemovalSelection merged; source_removal_selection_init(&merged, sources.allocator);
        size_t before = allocation_count;
        if (failure) { failure_at = before + failure; }
        SourceRemovalSelectionStatus status = source_removal_selection_merge(&merged, parts, 2);
        if (!failure) { assert(status == SOURCE_REMOVAL_SELECTION_OK); count = allocation_count - before; assert(count); }
        else { assert(status == SOURCE_REMOVAL_SELECTION_MEMORY && !merged.valid); }
        failure_at = 0; source_removal_selection_destroy(&merged);
    }
    SourceRemovalSelection merged; source_removal_selection_init(&merged, sources.allocator);
    assert(source_removal_selection_merge(&merged, NULL, 1) == SOURCE_REMOVAL_SELECTION_INVALID && !merged.attempted);
    assert(source_removal_selection_merge(&merged, parts, 0) == SOURCE_REMOVAL_SELECTION_INVALID && !merged.attempted);
    SourceRemovalSelection foreign = removal; SourceAnalysis foreign_analysis = analysis;
    foreign.analysis = &foreign_analysis; parts[1] = &foreign;
    assert(source_removal_selection_merge(&merged, parts, 2) == SOURCE_REMOVAL_SELECTION_INVALID && !merged.attempted);
    foreign = removal; SourceTreeSelection foreign_tree = selection; SourceRootCapture foreign_capture = capture;
    foreign_tree.capture = &foreign_capture; foreign.tree = &foreign_tree;
    assert(source_removal_selection_merge(&merged, parts, 2) == SOURCE_REMOVAL_SELECTION_INVALID && !merged.attempted);
    foreign = removal;
    AcquisitionUnitEntry changed = *removal.groups->files; changed.directory = !changed.directory; changed.next = NULL;
    SourceRemovalGroup group = {.files = &changed}; foreign.groups = &group;
    assert(source_removal_selection_merge(&merged, parts, 2) == SOURCE_REMOVAL_SELECTION_CHANGED && !merged.valid);
    assert(source_removal_selection_merge(&merged, parts, 2) == SOURCE_REMOVAL_SELECTION_INVALID);
    source_removal_selection_destroy(&merged); end_removal(MATERIAL_NONE);
}
