/** @file
 * @brief Verify child-only context, cyclic promotion and mandatory-test refusal behavior.
 * @see [T235 contract](../../../../../../../docs/implementation/acquisition-plan.md#t235-structural-source-and-test-delivery)
 */
/** A child-only selection preserves its draft parent as context without pulling
 * the parent's extra subtree asset, helper, tests or dependencies. */
static void it_closes_child_only_delivery_without_activating_its_draft_parent(void) {
    prepare(false, false, false, false); AcquisitionClosureProvider provider = {.expand = expand, .admit_test = admit};
    AcquisitionClosureStatus status = acquisition_closure_compute(&closure, &child_identity, 1, &provider);
    assert(status == ACQUISITION_CLOSURE_OK && closure.valid && expansions == 2 && admissions == 1);
    assert(retained("src/parent") && retained("src/parent.ini") && retained("src/parent+/child.extra"));
    assert(retained("tests/parent/child/check") && retained("package.ini") && retained("README.md"));
    assert(!retained("src/parent+/extra.data") && !retained("tests/parent/check") && !retained("src/unused"));
    assert(!retained("data.case"));
    assert(closure.tests && !closure.tests->next);
    finish();
}
/** A real reference edge promotes context, adds complete structural/test inputs,
 * and terminates the checked source cycle with one expansion per identity. */
static void it_promotes_context_and_closes_real_source_cycles_with_all_test_support(void) {
    prepare(true, false, false, false); AcquisitionClosureProvider provider = {.expand = expand, .admit_test = admit};
    const char *seeds[] = {child_identity, child_identity};
    AcquisitionClosureStatus status = acquisition_closure_compute(&closure, seeds, 2, &provider);
    assert(status == ACQUISITION_CLOSURE_OK && closure.valid && expansions == 5 && admissions == 2);
    assert(retained("src/parent+/extra.data") && retained("tests/parent/check") && retained("tests/parent/helper"));
    assert(!retained("src/unused") && closure.tests && closure.tests->next && !closure.tests->next->next);
    const AcquisitionRoleRecord *parent = closure.roles.records;
    while (parent && strcmp(parent->identity, parent_identity)) { parent = parent->next; }
    assert(parent && parent->expanded && parent->role == ACQUISITION_ACTIVE && parent->reasons && parent->reasons->next);
    finish();
}
/** A real independently selected test refusal stops publication and preserves its explanation. */
static void it_refuses_publication_after_independent_test_admission_failure(void) {
    prepare(false, false, false, false); refuse_test = true;
    AcquisitionClosureProvider provider = {.expand = expand, .admit_test = admit};
    AcquisitionClosureStatus status = acquisition_closure_compute(&closure, &child_identity, 1, &provider);
    assert(status == ACQUISITION_CLOSURE_PREPARATION_FAILED && !closure.valid && admissions == 0);
    assert(closure.issue.message && !strcmp(closure.issue.message, "Controlled independent test refusal"));
    finish();
}
/** A genuinely open test definition remains support and cannot satisfy the
 * source's mandatory direct concrete test requirement. */
static void it_refuses_an_associated_directory_containing_only_open_test_helpers(void) {
    prepare(false, true, false, false); AcquisitionClosureProvider provider = {.expand = expand, .admit_test = admit};
    AcquisitionClosureStatus status = acquisition_closure_compute(&closure, &child_identity, 1, &provider);
    assert(status == ACQUISITION_CLOSURE_MISSING_TEST && !closure.valid && admissions == 0);
    finish();
}
/** External required inputs enter only through an explicit captured group, with
 * their own source contract and a distinct declared-input reason. */
static void it_retains_explicit_declared_test_inputs_without_guessing_package_assets(void) {
    prepare(false, false, false, false); declared_data = true;
    AcquisitionClosureProvider provider = {.expand = expand, .admit_test = admit};
    AcquisitionClosureStatus status = acquisition_closure_compute(&closure, &child_identity, 1, &provider);
    assert(status == ACQUISITION_CLOSURE_OK && closure.valid && retained("data.case"));
    const AcquisitionMember *data = closure.members.first;
    while (data && !strstr(data->path, "/data.case")) { data = data->next; }
    assert(data && data->causes && data->causes->kind == ACQUISITION_DECLARED_INPUT && data->causes->source == origin);
    finish();
}
