/** @file
 * @brief Reject incomplete original ignored observations without substituting final captured bytes.
 * @see [T285 entry review](../../../../../../../docs/implementation/source-operations.md#t285-entry-review-and-execution)
 */
/** A final compiler response cannot supply missing original evidence. Each
 * omitted fact comes from a separate genuine, sealed original observer. */
static void it_requires_original_configuration_main_companion_and_parent_observations_for_reselection(void) {
    ReselectionFixture fixture; reselection_original(&fixture, false); reselection_observe(&fixture); reselection_image(&fixture);
    const size_t omitted[] = {1, 2, 4, 5};
    for (size_t variant = 0; variant < sizeof(omitted)/sizeof(*omitted); variant++) {
        SourceObservationRequest *requests = source_arena_allocate(&fixture.arena, fixture.request_count * sizeof(*requests)); assert(requests);
        size_t count = 0;
        for (size_t i = 0; i < fixture.request_count; i++) {
            if (i != fixture.material.count + omitted[variant]) { requests[count++] = fixture.requests[i]; }
        }
        SourceObservedImage incomplete; source_observed_image_init(&incomplete, sources.allocator);
        assert(source_observed_image_build(&incomplete, &capture, (SourceImageBounds){0}, requests, count) == SOURCE_OBSERVATION_OK);
        SourceCopyProofIssue issue;
        SourceCopyProofStatus status = source_copy_prove(&fixture.arena, &analysis, &incomplete, &fixture.mains, &fixture.files, NULL, NULL,
            &fixture.final, &fixture.client, removal.identities, removal.identity_count, &issue);
        if (variant == 1) {
            /* Exact child probing already observes its complete parent listing;
             * omitting a redundant directory request does not remove that fact. */
            assert(status == SOURCE_COPY_PROOF_OK);
        } else {
            assert(status == SOURCE_COPY_PROOF_INTRODUCTION);
            assert(issue.introduction_status == SOURCE_INTRODUCTION_MATERIAL && issue.path);
        }
        source_observed_image_destroy(&incomplete);
    }
    reselection_destroy(&fixture);
}
