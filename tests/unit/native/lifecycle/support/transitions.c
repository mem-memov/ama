/**
 * @file
 * @brief Verify forbidden lifecycle transitions, full-width child counters and exact first-error retention.
 */

/** Compare logical ledger fields, excluding C padding bytes. Rejected operations must change none. */
static void expect_same_lifetime(AmaLifetime actual, AmaLifetime expected) {
    assert(actual.phase == expected.phase && actual.constructor == expected.constructor);
    assert(actual.child_capacity == expected.child_capacity && actual.live_children == expected.live_children);
    assert(actual.native_live == expected.native_live && actual.cleanup_active == expected.cleanup_active);
}

/** Observe commands that are forbidden in every nonfresh phase, preserving all cleanup authority. */
static void expect_no_reconstruction(AmaLifetime *lifetime) {
    AmaLifetime before = *lifetime;
    assert(ama_lifetime_begin(lifetime, 0, AMA_CONSTRUCT_DEFAULT).code == AMA_INVALID_STATE);
    assert(ama_lifetime_finish(lifetime, true).code == AMA_INVALID_STATE);
    expect_same_lifetime(*lifetime, before);
}

/** Exercise rejected transitions before/during/after cleanup and consume each credit at most once. */
static void it_rejects_lifecycle_reentry_without_mutating_resource_credits(void) {
    AmaLifetime lifetime = {0};
    AmaLifetime before = lifetime;
    const AmaConstructorKind invalid[] = {(AmaConstructorKind)-1, (AmaConstructorKind)(AMA_CONSTRUCT_TRANSPORT + 1)};
    for (size_t index = 0; index < 2; index++) {
        assert(ama_lifetime_begin(&lifetime, 1, invalid[index]).code == AMA_INVALID_STATE);
        expect_same_lifetime(lifetime, before);
    }
    assert(ama_lifetime_child_ready(&lifetime).code == AMA_INVALID_STATE);
    assert(ama_lifetime_finish(&lifetime, true).code == AMA_INVALID_STATE);
    assert(ama_lifetime_begin_cleanup(&lifetime).code == AMA_INVALID_STATE);
    assert(ama_lifetime_end_cleanup(&lifetime).code == AMA_INVALID_STATE);
    size_t child = 91;
    assert(!ama_lifetime_take_native(&lifetime) && !ama_lifetime_next_child(&lifetime, &child) && child == 91);
    expect_same_lifetime(lifetime, before);
    assert(ama_lifetime_begin(&lifetime, 2, AMA_CONSTRUCT_CLONE).code == AMA_OK);
    expect_no_reconstruction(&lifetime);
    assert(ama_lifetime_end_cleanup(&lifetime).code == AMA_INVALID_STATE);
    assert(ama_lifetime_child_ready(&lifetime).code == AMA_OK);
    assert(ama_lifetime_child_ready(&lifetime).code == AMA_OK);
    assert(ama_lifetime_finish(&lifetime, true).code == AMA_OK);
    before = lifetime;
    expect_no_reconstruction(&lifetime);
    assert(ama_lifetime_child_ready(&lifetime).code == AMA_INVALID_STATE);
    assert(ama_lifetime_end_cleanup(&lifetime).code == AMA_INVALID_STATE);
    assert(!ama_lifetime_take_native(&lifetime) && !ama_lifetime_next_child(&lifetime, &child));
    expect_same_lifetime(lifetime, before);
    assert(ama_lifetime_begin_cleanup(&lifetime).code == AMA_OK);
    before = lifetime;
    expect_no_reconstruction(&lifetime);
    assert(ama_lifetime_begin_cleanup(&lifetime).code == AMA_INVALID_STATE);
    assert(ama_lifetime_child_ready(&lifetime).code == AMA_INVALID_STATE);
    assert(ama_lifetime_end_cleanup(&lifetime).code == AMA_INVALID_STATE);
    assert(!ama_lifetime_next_child(&lifetime, &child) && child == 91);
    expect_same_lifetime(lifetime, before);
    assert(ama_lifetime_take_native(&lifetime) && !ama_lifetime_take_native(&lifetime));
    assert(ama_lifetime_end_cleanup(&lifetime).code == AMA_INVALID_STATE);
    assert(ama_lifetime_next_child(&lifetime, &child) && child == 1);
    assert(ama_lifetime_end_cleanup(&lifetime).code == AMA_INVALID_STATE);
    assert(ama_lifetime_next_child(&lifetime, &child) && child == 0);
    assert(ama_lifetime_end_cleanup(&lifetime).code == AMA_OK);
    before = lifetime;
    expect_no_reconstruction(&lifetime);
    assert(ama_lifetime_begin_cleanup(&lifetime).code == AMA_INVALID_STATE);
    assert(ama_lifetime_end_cleanup(&lifetime).code == AMA_INVALID_STATE);
    assert(!ama_lifetime_take_native(&lifetime) && !ama_lifetime_next_child(&lifetime, &child) && child == 0);
    expect_same_lifetime(lifetime, before);
}

/** Directly instantiate a reachable boundary prefix without iterating SIZE_MAX times.
 * The ledger owns no resources; the mathematical prefix represents SIZE_MAX-1 completed children.
 */
static void it_bounds_child_credit_arithmetic_at_size_max(void) {
    AmaLifetime lifetime = {.phase = AMA_LIFETIME_CONSTRUCTING,
        .child_capacity = SIZE_MAX, .live_children = SIZE_MAX - 1};
    assert(ama_lifetime_child_ready(&lifetime).code == AMA_OK && lifetime.live_children == SIZE_MAX);
    AmaLifetime before = lifetime;
    assert(ama_lifetime_child_ready(&lifetime).code == AMA_INVALID_STATE);
    expect_same_lifetime(lifetime, before);
    assert(ama_lifetime_finish(&lifetime, false).code == AMA_OK);
    assert(ama_lifetime_begin_cleanup(&lifetime).code == AMA_OK);
    size_t child = 0;
    assert(ama_lifetime_next_child(&lifetime, &child) && child == SIZE_MAX - 1);
    assert(lifetime.live_children == SIZE_MAX - 1);
}

/** Distinct failing destructors all run, preserving the first exact code/message/length record. */
static void it_preserves_exact_primary_status_through_distinct_cleanup_failures(void) {
    static const char message[] = {'p','r','i','m','a','r','y'};
    for (size_t selected = 0; selected < 2; selected++) {
        Model model;
        Scenario scenario = {3, AMA_CONSTRUCT_SIZE, SIZE_MAX, 0};
        assert(model_construct(&model, scenario).code == AMA_OK);
        model.native.destroy_code = AMA_IO_FAILURE;
        model.children[2].destroy_code = AMA_NATIVE_FAILURE;
        model.children[1].destroy_code = AMA_OUT_OF_MEMORY;
        model.children[0].destroy_code = AMA_INVALID_STATE;
        AmaStatus primary = selected == 0 ? ama_status_success()
            : ama_status_make(AMA_INVALID_LITERAL, message, sizeof(message));
        AmaStatus status = model_cleanup(&model, primary);
        assert(status.code == (selected == 0 ? AMA_IO_FAILURE : primary.code));
        if (selected != 0) { assert(status.message == message && status.message_length == sizeof(message)); }
        assert(model.diagnostic_count == 4 && model.destruction_count == 4);
        const uint32_t codes[] = {AMA_IO_FAILURE,AMA_NATIVE_FAILURE,AMA_OUT_OF_MEMORY,AMA_INVALID_STATE};
        for (size_t index = 0; index < 4; index++) { assert(model.diagnostics[index].code == codes[index]); }
        expect_finalized(&model);
        assert(model_cleanup(&model, ama_status_success()).code == AMA_INVALID_STATE);
        assert(model.diagnostic_count == 4 && model.destruction_count == 4);
    }
}

/** Fail every real allocation in every constructor path, including the clone's temporary source. */
static void it_unwinds_every_actual_constructor_allocation_without_losing_resources(void) {
    for (int kind = AMA_CONSTRUCT_DEFAULT; kind <= AMA_CONSTRUCT_TRANSPORT; kind++) {
        for (size_t children = 0; children <= 3; children++) {
            Model model;
            Scenario scenario = {children, (AmaConstructorKind)kind, SIZE_MAX, 0};
            assert(model_construct(&model, scenario).code == AMA_OK);
            size_t count = allocation_count;
            assert(model_cleanup(&model, ama_status_success()).code == AMA_OK);
            expect_finalized(&model);
            for (size_t failed = 1; failed <= count; failed++) {
                assert(model_construct_with_failure(&model, scenario, failed).code == AMA_OUT_OF_MEMORY);
                assert(allocation_count == failed);
                expect_finalized(&model);
            }
        }
    }
}
