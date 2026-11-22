/** @file
 * @brief Verify explicitly selected draft units retain active children without invented parent tests or dependencies.
 * @see [T224 correction](../../../../../../../docs/implementation/source-operations.md)
 */
/** The known parent fixture is a production-parsed inactive main unless promotion
 * deliberately gives it an own checked definition. Mark that explicit fact for
 * boundary tests; suppress automatic parent test association only for the draft.
 */
static AcquisitionClosureStatus expand_draft(void *context, const char *identity, AcquisitionExpansion *output, SourceIssue *issue) {
    AcquisitionClosureStatus status = expand(context, identity, output, issue);
    if (status == ACQUISITION_CLOSURE_OK && !strcmp(identity, parent_identity)) {
        output->draft = true; output->test_directory = NULL; output->test_files = NULL;
    }
    return status;
}
/** Inject contradictory automatic test metadata without inventing a fake pointer. */
static AcquisitionClosureStatus expand_draft_with_tests(void *context, const char *identity, AcquisitionExpansion *output, SourceIssue *issue) {
    static const AcquisitionTestDirectory unsupported = {0};
    AcquisitionClosureStatus status = expand_draft(context, identity, output, issue);
    if (status == ACQUISITION_CLOSURE_OK && !strcmp(identity, parent_identity)) { output->test_directory = &unsupported; }
    return status;
}
/** Complete draft units keep opaque subtree bytes and independently active child experiments. */
static void it_retains_selected_draft_units_and_active_children_without_invented_parent_requirements(void) {
    prepare(false, false, false, false); AcquisitionClosureProvider provider = {.expand = expand_draft, .admit_test = admit};
    AcquisitionClosureStatus status = acquisition_closure_compute(&closure, &parent_identity, 1, &provider);
    assert(status == ACQUISITION_CLOSURE_OK && closure.valid && expansions == 3 && admissions == 1);
    assert(retained("src/parent") && retained("src/parent.ini") && retained("src/parent+/extra.data"));
    assert(retained("src/parent+/child.extra") && retained("tests/parent/child/check"));
    assert(!retained("tests/parent/check") && !retained("src/unused")); finish();
}
/** A checked own active definition or automatic parent test metadata contradicts Draft. */
static void it_refuses_contradictory_draft_definition_and_test_association_proofs(void) {
    prepare(true, false, false, false); AcquisitionClosureProvider provider = {.expand = expand_draft, .admit_test = admit};
    assert(acquisition_closure_compute(&closure, &parent_identity, 1, &provider) == ACQUISITION_CLOSURE_INVALID);
    assert(!closure.valid && admissions == 0 && closure.issue.message); finish();
    prepare(false, false, false, false); provider.expand = expand_draft_with_tests;
    assert(acquisition_closure_compute(&closure, &parent_identity, 1, &provider) == ACQUISITION_CLOSURE_INVALID);
    assert(!closure.valid && admissions == 0); finish();
}
/** A version is structural package scope, never a SYN-04 main. */
static AcquisitionClosureStatus expand_draft_version(void *context, const char *identity, AcquisitionExpansion *output, SourceIssue *issue) {
    AcquisitionClosureStatus status = expand_empty(context, identity, output, issue);
    if (status == ACQUISITION_CLOSURE_OK) { output->draft = true; }
    return status;
}
/** Reject a draft marker on whole-version facts before publishing memberships. */
static void it_refuses_a_draft_marker_on_selected_version_scope(void) {
    prepare_empty(); AcquisitionClosureProvider provider = {.expand = expand_draft_version, .admit_test = admit};
    assert(acquisition_closure_compute(&closure, &origin->version, 1, &provider) == ACQUISITION_CLOSURE_INVALID);
    assert(!closure.valid && !closure.members.first && admissions == 0); finish();
}
