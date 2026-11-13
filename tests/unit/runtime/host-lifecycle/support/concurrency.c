/**
 * @file
 * @brief Establish sequential context independence before probing rejected concurrent and callback entry.
 */

/** Two independently held execution leases share no root state or failure boundary. */
static void it_preserves_an_independent_execution_when_another_context_fails(void) {
    setup_execution();
    uint64_t first = create_context(), second = create_context();
    AmaContextReference a = execution_root(first), b = execution_root(second);
    AmaContextExecution left = {0}, right = {0};
    assert(ama_context_execution_begin(&registry, first, a, &execution_entry, &left).code == AMA_OK);
    assert(ama_context_execution_begin(&registry, second, b, &execution_entry, &right).code == AMA_OK);
    AmaContextView av = execution_view(&left), bv = execution_view(&right);
    assert(av.world != bv.world && av.program != bv.program && av.control != bv.control);
    execution_value(av)->native.number = 111;
    execution_value(bv)->native.number = 222;
    assert(return_execution(&left, true, 19).outcome == AMA_HOST_YIELDED);
    AmaContextOutcome outcome;
    assert(ama_context_execution_finish(&right, AMA_HOST_FAILED, ama_status_make(AMA_IO_FAILURE, NULL, 0), &outcome).code == AMA_OK);
    assert(outcome.outcome == AMA_HOST_FAILED && outcome.status.code == AMA_IO_FAILURE);
    assert(ama_context_dispose(&registry, second).code == AMA_IO_FAILURE);
    assert(ama_context_execution_begin(&registry, first, a, &execution_resume, &left).code == AMA_OK && left.selector == 19);
    assert(execution_value(execution_view(&left))->native.number == 111);
    assert(return_execution(&left, false, 0).outcome == AMA_HOST_COMPLETED);
    finish_contexts();
}

/** Host-only checked tokens and outcomes for a worker that must not reach any Ama allocator or hook. */
typedef struct {
    uint64_t identity;
    AmaContextReference root;
    AmaStatus admission;
    AmaStatus disposal;
} ExecutionThreadProbe;

/** Attempt one entry/disposal while the main thread holds that context's execution lease. */
static int execution_thread_attempt(void *argument) {
    ExecutionThreadProbe *probe = argument;
    AmaContextExecution execution = {0};
    probe->admission = ama_context_execution_begin(&registry, probe->identity, probe->root, &execution_entry, &execution);
    probe->disposal = ama_context_dispose(&registry, probe->identity);
    assert(execution.lease.context == NULL);
    return 0;
}

/** A context gate rejects another root, a worker thread and disposal without body or interface effects. */
static void it_refuses_concurrent_and_alternate_root_entry_without_changing_execution(void) {
    setup_execution();
    uint64_t identity = create_context();
    AmaContextReference root = execution_root(identity), other = execution_root(identity);
    AmaContextExecution execution = {0}, rejected = {0};
    assert(ama_context_execution_begin(&registry, identity, root, &execution_entry, &execution).code == AMA_OK);
    AmaContextView view = execution_view(&execution);
    execution_value(view)->native.number = 1234;
    size_t before = allocation_count, constructors = constructed;
    assert(ama_context_execution_begin(&registry, identity, other, &execution_entry, &rejected).code == AMA_BUSY);
    ExecutionThreadProbe probe = {.identity = identity, .root = root};
    thrd_t thread;
    assert(thrd_create(&thread, execution_thread_attempt, &probe) == thrd_success);
    int result;
    assert(thrd_join(thread, &result) == thrd_success && result == 0);
    assert(probe.admission.code == AMA_BUSY && probe.disposal.code == AMA_BUSY);
    assert(allocation_count == before && constructed == constructors && execution_value(view)->native.number == 1234);
    assert(view.control->phase == AMA_RUNTIME_EXECUTING);
    assert(return_execution(&execution, false, 0).outcome == AMA_HOST_COMPLETED);
    finish_contexts();
}

/** Host diagnostic callback state; tokens are explicit external control values, never native payload Ama references. */
typedef struct {
    uint64_t identity;
    AmaContextReference root;
    size_t attempts;
} ExecutionCallbackProbe;

/** Observe the primary report while the execution gate and active root metadata remain intact. */
static void execution_report_attempt(void *argument, AmaStatus status) {
    ExecutionCallbackProbe *probe = argument;
    report_status(NULL, status);
    AmaContextExecution rejected = {0};
    assert(ama_context_execution_begin(&registry, probe->identity, probe->root, &execution_entry, &rejected).code == AMA_BUSY);
    assert(ama_context_dispose(&registry, probe->identity).code == AMA_BUSY);
    probe->attempts++;
}

/** Failure reporting cannot reenter or dispose its still-active context; ordinary disposal follows cleanup. */
static void it_refuses_report_callback_reentry_until_accepted_failure_cleanup_finishes(void) {
    setup_execution();
    ExecutionCallbackProbe probe = {0};
    definition.report = execution_report_attempt;
    definition.report_context = &probe;
    probe.identity = create_context();
    probe.root = execution_root(probe.identity);
    AmaContextExecution execution = {0};
    assert(ama_context_execution_begin(&registry, probe.identity, probe.root, &execution_entry, &execution).code == AMA_OK);
    AmaContextOutcome outcome;
    assert(ama_context_execution_finish(&execution, AMA_HOST_FAILED, ama_status_make(AMA_NATIVE_FAILURE, NULL, 0), &outcome).code == AMA_OK);
    assert(probe.attempts == 1 && reports == 1 && outcome.outcome == AMA_HOST_FAILED);
    assert(ama_context_dispose(&registry, probe.identity).code == AMA_NATIVE_FAILURE);
    finish_contexts();
}
