/** @file
 * @brief Consume supplier introductions through the shared operation plan and explicit original client review.
 * @see [T285 entry review](../../../../../../../docs/implementation/source-operations.md#t285-entry-review-and-execution)
 */
/** Independently known empty final version permissions account for every actual
 * supplier. This is image/proof evidence, not supplemental-write authorization. */
static void it_proves_reselection_through_shared_plan_and_preserves_explicit_original_client_choices(void) {
    ReselectionFixture fixture; reselection_original(&fixture, false); reselection_observe(&fixture); reselection_image(&fixture);
    SourcePackageEnvironment expected; source_package_environment_init(&expected, sources.allocator);
    assert(source_package_environment_build(&expected, &fixture.final.catalog) == SOURCE_ENVIRONMENT_OK);
    SourceOperationConfigurationExpectation expectations[2]; size_t count = 0;
    for (const SourcePackageScope *scope = expected.first; scope; scope = scope->next) {
        assert(count < 2 && !scope->selection->count);
        expectations[count++] = (SourceOperationConfigurationExpectation){.environment = &expected, .scope = scope};
    }
    assert(count == 2);
    SourceClientRoot client = {.definition = definition("/vendor/pkg/v1.0.0/src/main"),
        .kind = SOURCE_CLIENT_ENTRY, .action = SOURCE_CLIENT_KEEP};
    SourceOperationPlanInputs inputs = {.original = &analysis, .observed = &fixture.observed,
        .final_roots = capture.roots, .final_root_count = capture.root_count,
        .mains = &fixture.mains, .files = &fixture.files, .removal = &removal,
        .changes = fixture.material.changes, .change_count = fixture.material.count,
        .configurations = expectations, .configuration_count = count};
    for (unsigned disposition = 0; disposition < 3; disposition++) {
        inputs.clients = disposition ? &client : NULL;
        client.action = disposition == 2 ? SOURCE_CLIENT_DROP : SOURCE_CLIENT_KEEP;
        SourceOperationPlan plan; source_operation_plan_init(&plan, sources.allocator);
        SourceOperationPlanStatus status = source_operation_plan_build(&plan, &inputs);
        if (disposition == 1) { assert(status == SOURCE_OPERATION_PLAN_REMOVAL && plan.removal_review.clients); }
        else {
            assert(status == SOURCE_OPERATION_PLAN_OK && plan.prepared);
            SourceCopyProofIssue issue;
            assert(source_operation_plan_prove(&plan, &fixture.client, &issue) == SOURCE_COPY_PROOF_OK);
        }
        source_operation_plan_destroy(&plan);
    }
    source_package_environment_destroy(&expected); reselection_destroy(&fixture);
}
