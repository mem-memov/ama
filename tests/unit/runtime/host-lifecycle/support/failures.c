/**
 * @file
 * @brief Fail every accepted host preparation allocation and reject incoherent kernel outcomes safely.
 */
#include "runtime/program/native/borrow.h"

/** A bad generated return becomes terminal failure, never a fabricated healthy host outcome. */
static void it_fails_incoherent_outcomes_and_malformed_status_after_accepted_execution(void) {
    for (size_t selected = 0; selected < 5; selected++) {
        setup_execution();
        uint64_t identity = create_context();
        AmaContextReference root = execution_root(identity);
        AmaContextExecution execution = {0};
        assert(ama_context_execution_begin(&registry, identity, root, &execution_entry, &execution).code == AMA_OK);
        AmaContextView view = execution_view(&execution);
        if (selected != 0) {
            uint64_t continuation = 99;
            assert(ama_program_complete(view.program, &view.control->current, &continuation).code == AMA_OK && continuation == 0);
        }
        const AmaHostOutcome outcomes[] = {AMA_HOST_COMPLETED,AMA_HOST_YIELDED,AMA_HOST_FAILED,AMA_HOST_COMPLETED,(AmaHostOutcome)99};
        AmaStatus status = selected == 3 ? ama_status_make(AMA_OK, NULL, 1) : ama_status_success();
        AmaContextOutcome outcome;
        assert(ama_context_execution_finish(&execution, outcomes[selected], status, &outcome).code == AMA_OK);
        assert(outcome.outcome == AMA_HOST_FAILED && outcome.root.context == 0 && outcome.status.code == AMA_INVALID_STATE);
        assert(live_allocations == 1 && reports == 1);
        assert(ama_context_dispose(&registry, identity).code == AMA_INVALID_STATE && live_allocations == 0);
        finish_contexts();
    }
}

/** Every measured allocation after admission yields Failed and frees all resources before explicit record disposal. */
static void it_distinguishes_accepted_preparation_failure_from_admission_rejection(void) {
    size_t count = 0;
    for (size_t selected = 0; selected <= count; selected++) {
        setup_execution();
        uint64_t identity = create_context();
        AmaContextReference root = execution_root(identity);
        size_t before = allocation_count;
        failure_at = selected == 0 ? 0 : before + selected;
        AmaContextExecution execution = {0};
        assert(ama_context_execution_begin(&registry, identity, root, &execution_entry, &execution).code == AMA_OK);
        failure_at = 0;
        assert(execution.lease.context != NULL);
        if (selected == 0) {
            count = allocation_count - before;
            assert(count != 0 && execution.status.code == AMA_OK);
            assert(return_execution(&execution, false, 0).outcome == AMA_HOST_COMPLETED);
            assert(ama_context_dispose(&registry, identity).code == AMA_OK);
        } else {
            assert(execution.status.code == AMA_OUT_OF_MEMORY);
            AmaContextOutcome outcome;
            assert(ama_context_execution_finish(&execution, AMA_HOST_FAILED, ama_status_success(), &outcome).code == AMA_OK);
            assert(outcome.outcome == AMA_HOST_FAILED && outcome.status.code == AMA_OUT_OF_MEMORY && outcome.root.context == 0);
            assert(live_allocations == 1 && reports == 1);
            assert(ama_context_dispose(&registry, identity).code == AMA_OUT_OF_MEMORY && live_allocations == 0);
        }
        finish_contexts();
    }
}

/** An active native view prevents host finalization until its linear scope is explicitly closed. */
static void it_keeps_execution_owned_until_native_borrows_and_invalid_finish_arguments_are_resolved(void) {
    setup_execution();
    uint64_t identity = create_context();
    AmaContextReference root = execution_root(identity);
    AmaContextExecution execution = {0};
    assert(ama_context_execution_begin(&registry, identity, root, &execution_entry, &execution).code == AMA_OK);
    AmaContextView view = execution_view(&execution);
    assert(ama_context_execution_finish(&execution, AMA_HOST_COMPLETED, ama_status_success(), NULL).code == AMA_INVALID_STATE);
    AmaNativeBorrowScope borrow = {0};
    assert(ama_native_borrow_begin(view.program, root.local, &borrow).code == AMA_OK);
    AmaContextOutcome unchanged = {.outcome = AMA_HOST_YIELDED, .root = root};
    assert(ama_context_execution_finish(&execution, AMA_HOST_COMPLETED, ama_status_success(), &unchanged).code == AMA_INVALID_STATE);
    assert(unchanged.outcome == AMA_HOST_YIELDED && unchanged.root.context == root.context);
    assert(execution.lease.context != NULL && view.world->operation_active && reports == 0);
    assert(ama_native_borrow_end(&borrow).code == AMA_OK);
    assert(return_execution(&execution, false, 0).outcome == AMA_HOST_COMPLETED);
    assert(ama_context_execution_finish(&execution, AMA_HOST_COMPLETED, ama_status_success(), &unchanged).code == AMA_INVALID_STATE);
    finish_contexts();
}
