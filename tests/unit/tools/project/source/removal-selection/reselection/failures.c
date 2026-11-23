/** @file
 * @brief Exhaust genuine supplier-introduction and full-proof scratch failures without changing owners or output.
 * @see [T285 entry review](../../../../../../../docs/implementation/source-operations.md#t285-entry-review-and-execution)
 */
/** A real original retirement and final compiler response anchor every failed
 * allocation prefix; neither source material nor valid flags are synthesized. */
static void it_retires_every_reselection_allocation_and_preserves_invalid_outputs(void) {
    ReselectionFixture fixture; reselection_original(&fixture, false); reselection_observe(&fixture); reselection_image(&fixture);
    SourceRegistryRetirement retired;
    assert(source_copy_registry_retirement(&fixture.arena, &capture, &fixture.final, removal.identities,
        removal.identity_count, &retired) == SOURCE_REGISTRY_OK);
    for (unsigned full = 0; full < 2; full++) {
        size_t total = 0, baseline = live_allocations;
        for (size_t failure = 0; !failure || failure <= total; failure++) {
            SourceArena arena; source_arena_init(&arena, sources.allocator);
            size_t before = allocation_count; if (failure) { failure_at = before + failure; }
            bool accepted;
            if (full) {
                SourceCopyProofIssue issue;
                accepted = source_copy_prove(&arena, &analysis, &fixture.observed, &fixture.mains, &fixture.files,
                    NULL, NULL, &fixture.final, &fixture.client, removal.identities, removal.identity_count, &issue) == SOURCE_COPY_PROOF_OK;
            } else {
                SourceCopyIntroduction output = {.count = SIZE_MAX}; SourceIntroductionIssue issue;
                SourceIntroductionStatus status = source_copy_introduction_prepare(&arena, &fixture.observed, &fixture.final,
                    &fixture.mains, retired, analysis.original.response.definitions, fixture.client.response.definitions, &output, &issue);
                accepted = status == SOURCE_INTRODUCTION_OK;
                if (failure) { assert(output.count == SIZE_MAX && status == SOURCE_INTRODUCTION_MEMORY); }
                else {
                    assert(output.count == 4 && output.conserved && !output.conserved->next);
                    assert(!strcmp(output.conserved->identity, "/vendor/other/v1.0.0/src/main"));
                }
            }
            if (!failure) { assert(accepted); total = allocation_count - before; assert(total); }
            else { assert(!accepted); }
            failure_at = 0; source_arena_destroy(&arena); assert(live_allocations == baseline);
        }
    }
    SourceCopyIntroduction output = {.count = SIZE_MAX}; SourceIntroductionIssue issue;
    assert(source_copy_introduction_prepare(NULL, &fixture.observed, &fixture.final, &fixture.mains, retired,
        analysis.original.response.definitions, fixture.client.response.definitions, &output, &issue) == SOURCE_INTRODUCTION_INVALID);
    const char *bad[] = {NULL}; SourceRegistryRetirement invalid = {bad, 1};
    assert(source_copy_introduction_prepare(&fixture.arena, &fixture.observed, &fixture.final, &fixture.mains, invalid,
        analysis.original.response.definitions, fixture.client.response.definitions, &output, &issue) == SOURCE_INTRODUCTION_INVALID);
    assert(output.count == SIZE_MAX);
    SourceCopyProofIssue proof;
    assert(source_copy_prove(&fixture.arena, &analysis, NULL, &fixture.mains, &fixture.files, NULL, NULL,
        &fixture.final, &fixture.client, removal.identities, removal.identity_count, &proof) == SOURCE_COPY_PROOF_INVALID);
    reselection_destroy(&fixture);
}
