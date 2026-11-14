/**
 * @file
 * @brief Verify explicit entry/resume outcomes, unchanged admission rejection and terminal context ownership.
 */
#include "common.c"
#include "support/failures.c"
#include "support/concurrency.c"
#include "support/diagnostics.c"
#include "support/start.c"

/** Entry resets interface values, resume preserves them, and completion removes resume authority. */
static void it_separates_entry_preparation_from_saved_position_resume(void) {
    setup_execution();
    uint64_t identity = create_context();
    AmaContextReference root = execution_root(identity);
    AmaContextLease lease = {0};
    AmaContextView view = enter_context(identity, &lease);
    RootValue *value = root_value(&lease, root);
    value->native.number = 123;
    execution_leaf(view, value->left)->number = 99;
    leave_context(&lease);
    size_t before = allocation_count;
    AmaContextExecution execution = {0};
    assert(ama_context_execution_begin(&registry, identity, root, &execution_resume, &execution).code == AMA_NO_RESUME);
    assert(execution.lease.context == NULL && allocation_count == before);
    assert(ama_context_execution_begin(&registry, identity, root, &execution_entry, &execution).code == AMA_OK);
    assert(execution.status.code == AMA_OK && execution.selector == 0);
    view = execution_view(&execution);
    value = execution_value(view);
    assert(value->native.number == 123 && execution_leaf(view, value->left)->number == 37);
    assert(execution_leaf(view, value->right)->number == 37);
    execution_leaf(view, value->right)->number = 88;
    AmaContextOutcome outcome = return_execution(&execution, true, 71);
    assert(outcome.outcome == AMA_HOST_YIELDED && outcome.root.context == identity && outcome.status.code == AMA_OK);
    assert(ama_handle_equal(outcome.root.local, root.local));
    before = allocation_count;
    assert(ama_context_execution_begin(&registry, identity, root, &execution_resume, &execution).code == AMA_OK);
    assert(execution.status.code == AMA_OK && execution.selector == 71 && allocation_count == before);
    view = execution_view(&execution);
    value = execution_value(view);
    assert(execution_leaf(view, value->right)->number == 88 && value->native.number == 123);
    AmaPoolCell *cell = NULL;
    assert(ama_program_cell(view.program, root.local, &cell).code == AMA_OK);
    assert(!cell->metadata.service.resume.valid && cell->metadata.service.return_link.host);
    outcome = return_execution(&execution, false, 0);
    assert(outcome.outcome == AMA_HOST_COMPLETED && outcome.status.code == AMA_OK);
    assert(ama_context_execution_begin(&registry, identity, root, &execution_resume, &execution).code == AMA_NO_RESUME);
    static const unsigned char literal = 42;
    AmaProgramInput input = {.field = 0, .source = {.kind = AMA_SOURCE_LITERAL, .type_id = 1, .literal = {&literal, 1}}};
    AmaProgramInvocation bound_entry = {.mode = AMA_ACCESS_ENTRY, .inputs = &input, .input_count = 1};
    assert(ama_context_execution_begin(&registry, identity, root, &bound_entry, &execution).code == AMA_OK);
    assert(execution.status.code == AMA_OK);
    view = execution_view(&execution);
    value = execution_value(view);
    assert(execution_leaf(view, value->left)->number == 42 && execution_leaf(view, value->right)->number == 42);
    assert(value->native.number == 123);
    (void)return_execution(&execution, false, 0);
    finish_contexts();
}

/** Foreign, empty, stale, invalid-mode and resume-with-input requests leave root state and history intact. */
static void it_rejects_invalid_host_admission_without_changing_healthy_context_state(void) {
    setup_execution();
    uint64_t first = create_context(), second = create_context();
    AmaContextReference root = execution_root(first), foreign = execution_root(second);
    assert(ama_handle_equal(root.local, foreign.local));
    AmaContextLease lease = {0};
    AmaContextView view = enter_context(first, &lease);
    RootValue *value = root_value(&lease, root);
    value->native.number = 456;
    AmaPoolCell *cell = NULL;
    assert(ama_program_cell(view.program, root.local, &cell).code == AMA_OK);
    cell->metadata.service.last_refusal = ama_status_make(AMA_BUSY, NULL, 0);
    leave_context(&lease);
    size_t before = allocation_count;
    AmaContextExecution execution = {0};
    assert(ama_context_execution_begin(&registry, first, foreign, &execution_entry, &execution).code == AMA_TYPE_MISMATCH);
    assert(ama_context_execution_begin(&registry, first, (AmaContextReference){0}, &execution_entry, &execution).code == AMA_EMPTY_REFERENCE);
    AmaContextReference stale = root;
    stale.local.generation++;
    assert(ama_context_execution_begin(&registry, first, stale, &execution_entry, &execution).code == AMA_STALE_REFERENCE);
    AmaProgramInvocation invalid = {.mode = AMA_ACCESS_READ};
    assert(ama_context_execution_begin(&registry, first, root, &invalid, &execution).code == AMA_INVALID_STATE);
    AmaProgramInput input = {0};
    invalid = (AmaProgramInvocation){.mode = AMA_ACCESS_RESUME, .inputs = &input, .input_count = 1};
    assert(ama_context_execution_begin(&registry, first, root, &invalid, &execution).code == AMA_INVALID_STATE);
    assert(execution.lease.context == NULL && execution.root.context == 0 && allocation_count == before);
    view = enter_context(first, &lease);
    assert(root_value(&lease, root)->native.number == 456);
    assert(ama_program_cell(view.program, root.local, &cell).code == AMA_OK);
    assert(cell->metadata.service.last_refusal.code == AMA_BUSY && cell->metadata.service.activity == AMA_INSTANCE_READY);
    assert(view.control->phase == AMA_RUNTIME_IDLE);
    leave_context(&lease);
    finish_contexts();
}

/** Accepted technical failure cleans once, preserves first status and retains only a diagnostic record. */
static void it_cleans_failed_execution_and_prevents_resume_before_explicit_context_disposal(void) {
    setup_execution();
    uint64_t identity = create_context();
    AmaContextReference root = execution_root(identity);
    AmaContextExecution execution = {0};
    assert(ama_context_execution_begin(&registry, identity, root, &execution_entry, &execution).code == AMA_OK);
    static const char message[] = {'b','o','d','y'};
    failed_destructor = destroyed + 1;
    AmaContextOutcome outcome;
    assert(ama_context_execution_finish(&execution, AMA_HOST_FAILED,
        ama_status_make(AMA_NATIVE_FAILURE, message, sizeof(message)), &outcome).code == AMA_OK);
    assert(outcome.outcome == AMA_HOST_FAILED && outcome.root.context == 0);
    assert(outcome.status.code == AMA_NATIVE_FAILURE && outcome.status.message == message && outcome.status.message_length == sizeof(message));
    assert(reports == 2 && live_allocations == 1);
    size_t cleaned = destroyed;
    assert(ama_context_execution_begin(&registry, identity, root, &execution_resume, &execution).code == AMA_INVALID_STATE);
    AmaContextLease lease = {0};
    AmaContextView view = enter_context(identity, &lease);
    assert(view.control->phase == AMA_RUNTIME_TERMINATED && ama_handle_is_empty(view.control->root));
    assert(view.world->type_count == 0 && view.world->types == NULL);
    assert(view.program->constant_count == 0 && view.program->constants == NULL);
    assert(view.status.code == AMA_NATIVE_FAILURE);
    AmaObjectType *unpublished_type = NULL;
    AmaPoolCell *unpublished_cell = NULL;
    assert(ama_context_resolve(&lease, root, AMA_ACCESS_READ, &unpublished_type, &unpublished_cell).code == AMA_INVALID_STATE);
    assert(unpublished_type == NULL && unpublished_cell == NULL);
    leave_context(&lease);
    assert(ama_context_dispose(&registry, identity).code == AMA_NATIVE_FAILURE && destroyed == cleaned && live_allocations == 0 && reports == 2);
    finish_contexts();
}

/** A yielded root is destroyed without dispatching its saved label or constructing another value. */
static void it_disposes_yielded_and_empty_contexts_without_running_saved_code(void) {
    setup_execution();
    uint64_t empty = create_context();
    size_t before = constructed;
    assert(ama_context_dispose(&registry, empty).code == AMA_OK && constructed == before);
    uint64_t identity = create_context();
    AmaContextReference root = execution_root(identity);
    AmaContextExecution execution = {0};
    assert(ama_context_execution_begin(&registry, identity, root, &execution_entry, &execution).code == AMA_OK);
    assert(return_execution(&execution, true, 17).outcome == AMA_HOST_YIELDED);
    before = constructed;
    assert(ama_context_dispose(&registry, identity).code == AMA_OK && constructed == before);
    assert(ama_context_execution_begin(&registry, identity, root, &execution_resume, &execution).code == AMA_STALE_REFERENCE);
    finish_contexts();
}

/** Run host-control cases explicitly; generated label dispatch is a later integration boundary. */
void run(void) {
    it_starts_an_explicit_root_once_under_the_execution_lease();
    it_cleans_every_failed_first_root_construction_without_publishing_a_root();
    it_reports_the_active_execution_chain_before_releasing_all_context_resources();
    it_preserves_an_independent_execution_when_another_context_fails();
    it_distinguishes_accepted_preparation_failure_from_admission_rejection();
    it_fails_incoherent_outcomes_and_malformed_status_after_accepted_execution();
    it_keeps_execution_owned_until_native_borrows_and_invalid_finish_arguments_are_resolved();
    it_refuses_concurrent_and_alternate_root_entry_without_changing_execution();
    it_refuses_report_callback_reentry_until_accepted_failure_cleanup_finishes();
    it_separates_entry_preparation_from_saved_position_resume();
    it_rejects_invalid_host_admission_without_changing_healthy_context_state();
    it_cleans_failed_execution_and_prevents_resume_before_explicit_context_disposal();
    it_disposes_yielded_and_empty_contexts_without_running_saved_code();
}

/** Succeed only after every admission/outcome and counted resource observation passes. */
int main(void) { run(); return EXIT_SUCCESS; }
