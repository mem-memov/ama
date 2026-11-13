/**
 * @file
 * @brief Observe accepted host failure with an intact nested Ama chain before allocation-free context cleanup.
 */
#include <unistd.h>

/** Real acquire/prepare/connect transitions produce the nested trace; failure cleanup must not allocate or report twice. */
static void it_reports_the_active_execution_chain_before_releasing_all_context_resources(void) {
    setup_execution();
    const AmaProgramLocation locations[] = {
        {.operation = 73, .path = "callee", .line = 6, .column = 4, .role = "native entry", .type = ".callee"},
        {.operation = 74, .path = "caller", .line = 3, .column = 2, .role = "invocation", .type = ".caller"}
    };
    definition.report = NULL;
    definition.locations = locations;
    definition.location_count = 2;
    uint64_t identity = create_context();
    AmaContextReference caller = execution_root(identity), callee = execution_root(identity);
    AmaContextLease lease = {0};
    AmaContextView view = enter_context(identity, &lease);
    root_value(&lease, caller)->self = callee.local;
    leave_context(&lease);
    AmaContextExecution execution = {0};
    assert(ama_context_execution_begin(&registry, identity, caller, &execution_entry, &execution).code == AMA_OK);
    view = execution_view(&execution);
    const AmaProgramAccess target = {.field = 4};
    const AmaProgramInvocation invocation = {.target = &target, .mode = AMA_ACCESS_ENTRY};
    bool refused = true;
    assert(ama_program_acquire(view.program, caller.local, &invocation, &refused).code == AMA_OK && !refused);
    assert(ama_program_prepare(view.program, caller.local, &invocation).code == AMA_OK);
    assert(ama_program_connect(view.program, &view.control->current, 101, 74).code == AMA_OK);
    AmaPoolCell *caller_cell = NULL, *callee_cell = NULL;
    assert(ama_program_cell(view.program, caller.local, &caller_cell).code == AMA_OK);
    assert(ama_program_cell(view.program, callee.local, &callee_cell).code == AMA_OK);
    caller_cell->metadata.service.current_operation = 74;
    callee_cell->metadata.service.current_operation = 73;
    FILE *capture = tmpfile();
    assert(capture != NULL && fflush(stderr) == 0);
    int saved = dup(STDERR_FILENO);
    assert(saved >= 0 && dup2(fileno(capture), STDERR_FILENO) >= 0);
    size_t before = allocation_count;
    failure_at = before + 1;
    AmaContextOutcome outcome;
    assert(ama_context_execution_finish(&execution, AMA_HOST_FAILED,
        ama_status_make(AMA_NATIVE_FAILURE, "primary", 7), &outcome).code == AMA_OK);
    failure_at = 0;
    assert(fflush(stderr) == 0 && dup2(saved, STDERR_FILENO) >= 0 && close(saved) == 0);
    assert(allocation_count == before && live_allocations == 1);
    rewind(capture);
    char text[1024];
    size_t count = fread(text, 1, sizeof(text) - 1, capture);
    assert(count < sizeof(text) - 1 && !ferror(capture) && fclose(capture) == 0);
    text[count] = '\0';
    const char *first = strstr(text, "callee:6:4"), *second = strstr(text, "caller:3:2");
    assert(strstr(text, "Ama runtime failure: 4: primary") != NULL);
    assert(first != NULL && second != NULL && first < second && strstr(second + 1, "caller:3:2") == NULL);
    assert(outcome.outcome == AMA_HOST_FAILED && outcome.status.code == AMA_NATIVE_FAILURE);
    assert(ama_context_dispose(&registry, identity).code == AMA_NATIVE_FAILURE);
    finish_contexts();
}
