/** @file
 * @brief Fail each scalar shared constructor/allocation prefix and all reverse cleanup hook positions without publishing a partial context.
 */
/** A failed dependency closure publishes no context/root, cleans its complete native prefix, and leaves the registry able to create a fresh isolated context. */
static void it_publishes_no_context_or_root_when_each_shared_native_constructor_fails(void) {
    for (size_t failure = 1; failure <= 7; failure++) {
        setup_shared();
        failed_constructor = failure;
        uint64_t identity = 0;
        AmaStatus status = ama_context_create(&registry, &definition, &identity);
        assert(status.code == AMA_INVALID_LITERAL && identity == 0 && registry.first == NULL && registry.pending == 0);
        assert(constructed == failure && destroyed == failure - 1);
        identity = create_context();
        AmaContextLease lease = {0};
        AmaContextView view = enter_context(identity, &lease);
        assert_no_instances(view);
        assert_shared_values(view);
        leave_context(&lease);
        status = ama_context_dispose(&registry, identity);
        assert(status.code == AMA_OK && constructed == destroyed + 1);
        finish_contexts();
    }
}
/** All allocations for descriptors, shared child arrays, payloads and native resources unwind without a published identity or a default instance. */
static void it_cleans_every_shared_factory_allocation_prefix_and_allows_a_fresh_retry(void) {
    setup_shared();
    size_t before = allocation_count;
    uint64_t identity = create_context();
    size_t count = allocation_count - before;
    AmaStatus status = ama_context_dispose(&registry, identity);
    assert(status.code == AMA_OK);
    finish_contexts();
    bool native_work = false;
    for (size_t failure = 1; failure <= count; failure++) {
        setup_shared();
        failure_at = allocation_count + failure;
        identity = 0;
        status = ama_context_create(&registry, &definition, &identity);
        assert(status.code == AMA_OUT_OF_MEMORY && identity == 0 && registry.first == NULL && registry.pending == 0);
        native_work |= constructed != 0;
        failure_at = 0;
        identity = create_context();
        AmaContextLease lease = {0};
        AmaContextView view = enter_context(identity, &lease);
        assert_no_instances(view);
        assert_shared_values(view);
        leave_context(&lease);
        status = ama_context_dispose(&registry, identity);
        assert(status.code == AMA_OK);
        finish_contexts();
    }
    assert(native_work);
}
/** Each reverse shared destructor may fail: remaining credits still finalize once, the context stays disposed, and repeated disposal cannot rerun hooks. */
static void it_finishes_reverse_shared_cleanup_and_preserves_the_first_destructor_error(void) {
    for (size_t failure = 1; failure <= 7; failure++) {
        setup_shared();
        uint64_t identity = create_context();
        AmaContextLease lease = {0};
        AmaContextView view = enter_context(identity, &lease);
        uintptr_t expected[7];
        expected_shared_cleanup(view, expected);
        assert_no_instances(view);
        leave_context(&lease);
        failed_destructor = failure;
        AmaStatus status = ama_context_dispose(&registry, identity);
        assert(status.code == AMA_IO_FAILURE && cleanup_count == 7 && destroyed == 7 && reports == 1);
        for (size_t index = 0; index < 7; index++) { assert(cleanup_addresses[index] == expected[index]); }
        status = ama_context_dispose(&registry, identity);
        assert(status.code == AMA_STALE_REFERENCE && cleanup_count == 7);
        finish_contexts();
    }
}
