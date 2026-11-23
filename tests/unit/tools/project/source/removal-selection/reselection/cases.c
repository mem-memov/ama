/** @file
 * @brief Check untouched supplier acceptance and refusal of original target/client substitution.
 * @see [T285 entry review](../../../../../../../docs/implementation/source-operations.md#t285-entry-review-and-execution)
 */
/** Independent final selection permits an unchanged supplier, while an outside
 * original edge still belongs to its original target even if the name is reused. */
static void it_accepts_untouched_external_reselection_and_refuses_original_target_substitution(void) {
    for (unsigned profile = 0; profile < 3; profile++) {
        ReselectionFixture fixture; reselection_original(&fixture, profile);
        reselection_observe(&fixture); reselection_image(&fixture);
        SourceCopyProofIssue issue;
        SourceCopyProofStatus status = source_copy_prove(&fixture.arena, &analysis, &fixture.observed, &fixture.mains,
            &fixture.files, NULL, NULL, &fixture.final, &fixture.client, removal.identities, removal.identity_count, &issue);
        assert(status == (profile ? SOURCE_COPY_PROOF_INTRODUCTION : SOURCE_COPY_PROOF_OK));
        if (profile == 1) { assert(issue.introduction_status == SOURCE_INTRODUCTION_TARGET && issue.introduction.edge); }
        if (profile == 2) { assert(issue.introduction_status == SOURCE_INTRODUCTION_ORIGIN); }
        reselection_destroy(&fixture);
    }
}
