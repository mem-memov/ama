/**
 * @file
 * @brief Observe checked streaming diagnostics with an intact caller chain and no runtime heap.
 *
 * Included by the context leaf after its counted fixtures. A temporary host stream
 * is prepared before runtime allocation is disabled; no generated source is inspected.
 */
#include <unistd.h>

/** Normalize malformed transport before message access and stream the existing caller chain.
 * A later cleanup error stays secondary; neither reporting operation allocates runtime storage,
 * destroys caller links nor replaces the initiating caller's saved refusal.
 */
static void it_streams_checked_diagnostics_without_runtime_allocation(void) {
    setup_contexts();
    uint64_t identity = create_context();
    AmaContextLease lease = {0};
    AmaContextView view = enter_context(identity, &lease);
    AmaContextReference caller = create_root(&lease, view), callee = create_root(&lease, view);
    AmaPoolCell *caller_cell = NULL, *callee_cell = NULL;
    AmaStatus status = ama_pool_lookup(&view.world->types[1]->pool, caller.local, false, &caller_cell);
    assert(status.code == AMA_OK);
    status = ama_pool_lookup(&view.world->types[1]->pool, callee.local, false, &callee_cell);
    assert(status.code == AMA_OK);
    caller_cell->metadata.service.current_operation = 74;
    caller_cell->metadata.service.last_refusal = ama_status_make(AMA_BUSY, NULL, 0);
    callee_cell->metadata.service.current_operation = 73;
    callee_cell->metadata.service.return_link = (AmaReturnLink){true, false, caller.local, 1, 74};
    const AmaProgramLocation locations[] = {
        {.operation = 73, .path = "callee", .line = 6, .column = 4, .role = "native entry", .type = ".callee"},
        {.operation = 74, .path = "caller", .line = 3, .column = 2, .role = "invocation", .type = ".caller"}
    };
    view.program->locations = locations;
    view.program->location_count = 2;
    view.program->current = callee.local;
    FILE *capture = tmpfile();
    assert(capture != NULL && fflush(stderr) == 0);
    int saved = dup(STDERR_FILENO);
    assert(saved >= 0 && dup2(fileno(capture), STDERR_FILENO) >= 0);
    size_t before = allocation_count;
    failure_at = before + 1;
    ama_program_report(view.program, ama_status_make(AMA_OK, NULL, 1));
    ama_program_report(view.program, ama_status_make(AMA_IO_FAILURE, "cleanup", 7));
    failure_at = 0;
    int flushed = fflush(stderr);
    int restored = dup2(saved, STDERR_FILENO);
    int closed = close(saved);
    assert(flushed == 0 && restored >= 0 && closed == 0 && allocation_count == before);
    rewind(capture);
    char text[1024];
    size_t length = fread(text, 1, sizeof(text) - 1, capture);
    assert(!ferror(capture) && length < sizeof(text) - 1 && fclose(capture) == 0);
    text[length] = '\0';
    assert(strstr(text, "Ama runtime failure: 5: Malformed native status") != NULL);
    const char *first = strstr(text, "callee:6:4"), *second = strstr(text, "caller:3:2");
    const char *secondary = strstr(text, "Secondary Ama runtime failure: 13: cleanup");
    assert(first != NULL && second != NULL && secondary != NULL && first < second && second < secondary);
    assert(strstr(secondary, "callee:6:4") == NULL && view.program->failure_reported);
    assert(caller_cell->metadata.service.last_refusal.code == AMA_BUSY);
    assert(ama_handle_equal(callee_cell->metadata.service.return_link.caller, caller.local));
    leave_context(&lease);
    finish_contexts();
}
