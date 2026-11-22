/** @file
 * @brief Verify injected test types, empty selections and malformed request refusal.
 * @see [T235 contract](../../../../../../../docs/implementation/acquisition-plan.md#t235-structural-source-and-test-delivery)
 */
/** Concrete test-only types may instantiate a src template without creating an
 * explicit forbidden src-to-tests definition dependency or tests-of-tests tree. */
static void it_retains_injected_test_types_through_their_original_test_scope(void) {
    prepare(false, false, true, false);
    AcquisitionClosureProvider provider = {.expand = expand, .admit_test = admit};
    AcquisitionClosureStatus status = acquisition_closure_compute(&closure, &child_identity, 1, &provider);
    assert(status == ACQUISITION_CLOSURE_OK && closure.valid && expansions == 3 && admissions == 1);
    assert(retained("tests/parent/child/support/shape") && !retained("src/unused"));
    const DeliverySourceDefinition *child = definitions;
    while (child && strcmp(child->identity, child_identity)) { child = child->next; }
    assert(child && child->kind == DELIVERY_SOURCE_PARAMETERIZED && !child->dependencies);
    assert(closure.tests && !closure.tests->next);
    finish();
}
/** Empty selections are stable fixed points; malformed roots and incomplete
 * provider requests refuse before source preparation or callback invocation. */
static void it_handles_empty_and_invalid_selection_requests_without_provider_effects(void) {
    setup_test(); AcquisitionClosureProvider provider = {.expand = expand, .admit_test = admit};
    acquisition_closure_init(&closure, sources.allocator);
    AcquisitionClosureStatus status = acquisition_closure_compute(&closure, NULL, 0, &provider);
    assert(status == ACQUISITION_CLOSURE_OK && closure.valid && !closure.roles.records && !closure.members.first);
    status = acquisition_closure_compute(&closure, NULL, 0, &provider); assert(status == ACQUISITION_CLOSURE_INVALID);
    acquisition_closure_destroy(&closure); acquisition_closure_init(&closure, sources.allocator);
    const char *bad = "relative module";
    status = acquisition_closure_compute(&closure, &bad, 1, &provider);
    assert(status == ACQUISITION_CLOSURE_INVALID && !closure.valid && !closure.roles.records);
    acquisition_closure_destroy(&closure); acquisition_closure_init(&closure, sources.allocator);
    status = acquisition_closure_compute(&closure, NULL, 0, NULL);
    assert(status == ACQUISITION_CLOSURE_INVALID && !closure.valid);
    acquisition_closure_destroy(&closure); destroy_test();
}
/** Internal typed evidence closes a direct test's own result formal, so its
 * original parameterized definition must still become a concrete experiment. */
static void it_admits_tests_whose_original_formals_close_from_typed_defaults(void) {
    prepare(false, false, false, true);
    AcquisitionClosureProvider provider = {.expand = expand, .admit_test = admit};
    AcquisitionClosureStatus status = acquisition_closure_compute(&closure, &child_identity, 1, &provider);
    assert(status == ACQUISITION_CLOSURE_OK && closure.valid && expansions == 3 && admissions == 1);
    assert(closure.tests && !closure.tests->next && closure.tests->definition->kind == DELIVERY_SOURCE_PARAMETERIZED);
    assert(retained("tests/parent/child/support/palette")); finish();
}
