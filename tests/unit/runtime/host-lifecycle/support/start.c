/** @file
 * @brief Verify explicit first-root execution constructs its interface once and owns every failed construction prefix.
 */
/** Fresh startup differs from restarting an existing root: literal and result-copy constructors must each run once. */
static void it_starts_an_explicit_root_once_under_the_execution_lease(void) {
    setup_execution();
    uint64_t identity = create_context();
    size_t before = constructed, copies = cloned;
    AmaContextExecution execution = {0};
    AmaStatus status = ama_context_execution_start(&registry, identity, 0, &execution);
    assert(status.code == AMA_TYPE_MISMATCH && execution.lease.context == NULL && constructed == before);
    status = ama_context_execution_start(&registry, identity, 2, &execution);
    assert(status.code == AMA_OK && execution.status.code == AMA_OK);
    assert(execution.root.context == identity && execution.root.local.generation != 0);
    assert(constructed == before + 3 && cloned == copies + 1);
    AmaContextView view = execution_view(&execution);
    RootValue *value = execution_value(view);
    assert(execution_leaf(view, value->left)->number == 37 && execution_leaf(view, value->right)->number == 37);
    AmaContextExecution competing = {0};
    status = ama_context_execution_start(&registry, identity, 2, &competing);
    assert(status.code == AMA_BUSY && competing.lease.context == NULL);
    AmaContextOutcome outcome = return_execution(&execution, false, 0);
    assert(outcome.outcome == AMA_HOST_COMPLETED && outcome.status.code == AMA_OK);
    before = allocation_count;
    status = ama_context_execution_start(&registry, identity, 2, &execution);
    assert(status.code == AMA_INVALID_STATE && execution.lease.context == NULL && allocation_count == before);
    finish_contexts();
}

/** Once startup is admitted, every allocation failure is an execution failure whose empty or published root remains owned until finish. */
static void it_cleans_every_failed_first_root_construction_without_publishing_a_root(void) {
    setup_execution();
    uint64_t identity = create_context();
    AmaContextExecution execution = {0};
    size_t before = allocation_count;
    AmaStatus status = ama_context_execution_start(&registry, identity, 2, &execution);
    assert(status.code == AMA_OK && execution.status.code == AMA_OK);
    size_t count = allocation_count - before;
    (void)return_execution(&execution, false, 0);
    finish_contexts();
    for (size_t failure = 1; failure <= count; failure++) {
        setup_execution();
        identity = create_context();
        execution = (AmaContextExecution){0};
        failure_at = allocation_count + failure;
        status = ama_context_execution_start(&registry, identity, 2, &execution);
        failure_at = 0;
        assert(status.code == AMA_OK && execution.status.code == AMA_OUT_OF_MEMORY);
        assert(execution.lease.context != NULL && execution.root.context == identity);
        assert(ama_handle_is_empty(execution.root.local));
        AmaContextOutcome outcome;
        status = ama_context_execution_finish(&execution, AMA_HOST_FAILED, execution.status, &outcome);
        assert(status.code == AMA_OK && outcome.status.code == AMA_OUT_OF_MEMORY);
        assert(outcome.root.context == 0 && execution.lease.context == NULL);
        status = ama_context_registry_destroy(&registry);
        assert(status.code == AMA_OUT_OF_MEMORY);
        destroy_objects();
    }
}
