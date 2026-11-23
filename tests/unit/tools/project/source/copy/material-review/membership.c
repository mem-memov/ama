/** @file
 * @brief Reject a complete-looking compiled image whose P omits required test-directory provenance.
 * @see [T276 checkpoint](../../../../../../../docs/implementation/source-operations.md#t276-remaining-implementation-steps-after-t283)
 */
/** A test main survives compilation when its parent is silently regenerated as
 * scaffolding. Required association membership must still occur in the actual
 * copy relation, preserving its captured origin rather than guessing metadata. */
static void check_material_review_missing_test_directory(const SourceOperationPlan *accepted, const SourceOperationIntent *intent) {
    const AcquisitionMember *member = accepted->inputs.copy->closure.members.first;
    for (; member; member = member->next) {
        if (!member->directory) { continue; }
        const AcquisitionMemberCause *cause = member->causes;
        while (cause && cause->kind != ACQUISITION_TEST_MEMBER) { cause = cause->next; }
        if (cause) { break; }
    }
    assert(member);
    const SourceFileMapping *omitted = accepted->inputs.files->first;
    while (omitted && strcmp(omitted->source, member->path)) { omitted = omitted->next; }
    assert(omitted && omitted->original->directory);
    SourceFileMap incomplete; source_file_map_init(&incomplete, sources.allocator);
    for (const SourceFileMapping *row = accepted->inputs.files->first; row; row = row->next) {
        if (row == omitted) { continue; }
        const SourceFileMapping *added = NULL;
        assert(source_file_map_add(&incomplete, row->original, row->destination, &added) == SOURCE_FILE_MAP_OK);
    }
    assert(source_file_map_seal(&incomplete) == SOURCE_FILE_MAP_OK);
    SourceOperationPlanInputs inputs = accepted->inputs; inputs.files = &incomplete;
    SourceOperationPlan image; source_operation_plan_init(&image, sources.allocator);
    assert(source_operation_plan_build(&image, &inputs) == SOURCE_OPERATION_PLAN_OK);
    SourceOperationIntentIssue provenance;
    assert(source_operation_intent_check(&image, intent, &provenance) == SOURCE_OPERATION_INTENT_OK);
    FILE *request = fopen("generated/material-without-test-directory.json", "wb"); assert(request);
    assert(delivery_source_snapshot_write(request, &image.capture.view) && fclose(request) == 0);
    SourceCompilerClient client; source_compiler_client_init(&client, sources.allocator);
    assert(source_compiler_client_run(&client, "../../../../../../ama", "generated/material-without-test-directory.json",
        &image.capture.view, NULL) == SOURCE_COMPILER_OK);
    SourceCopyProofIssue proof;
    assert(source_operation_plan_prove(&image, &client, &proof) == SOURCE_COPY_PROOF_OK);
    SourceOperationMaterialReview review; source_operation_material_init(&review, sources.allocator);
    assert(source_operation_material_build(&review, &image, intent) == SOURCE_OPERATION_MATERIAL_FILESET && !review.valid);
    assert(!strcmp(review.failed_path, omitted->destination));
    source_operation_material_destroy(&review); source_compiler_client_destroy(&client);
    source_operation_plan_destroy(&image); source_file_map_destroy(&incomplete);
}
