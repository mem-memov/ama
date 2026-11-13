/**
 * @file
 * @brief Bind declared parameter/result defaults and explicit host-root return helpers to isolated contexts.
 */
#ifndef AMA_HOST_LIFECYCLE_TEST_COMMON_C
#define AMA_HOST_LIFECYCLE_TEST_COMMON_C
#include "../contexts/common.c"
#include "runtime/context/execution/execution.h"

static const AmaProgramAccess execution_input_access = {.field = 0};
static const AmaProgramSource execution_input_source = {
    .kind = AMA_SOURCE_LOCAL, .type_id = 1, .access = &execution_input_access
};
static const AmaProgramInvocation execution_entry = {.mode = AMA_ACCESS_ENTRY};
static const AmaProgramInvocation execution_resume = {.mode = AMA_ACCESS_RESUME};

/** Establish immutable parameter/result declarations before any context starts borrowing them. */
static void setup_execution(void) {
    setup_contexts();
    context_root_fields[0] = (AmaProgramField){.role = AMA_PROGRAM_PARAMETER,
        .initializers = &context_literal, .initializer_count = 1, .operation = 21};
    context_root_fields[1] = (AmaProgramField){.role = AMA_PROGRAM_RESULT,
        .initializers = &execution_input_source, .initializer_count = 1, .operation = 22};
}

/** Create and wrap one explicit root under a short context lease; no Ama statement runs. */
static AmaContextReference execution_root(uint64_t identity) {
    AmaContextLease lease = {0};
    AmaContextView view = enter_context(identity, &lease);
    AmaContextReference root = create_root(&lease, view);
    leave_context(&lease);
    return root;
}

/** Borrow kernel owners while the accepted execution pins its context. */
static AmaContextView execution_view(AmaContextExecution *execution) {
    AmaContextView view;
    assert(ama_context_inspect(&execution->lease, &view).code == AMA_OK);
    return view;
}

/** Observe existing receiver storage with private kernel authority, without issuing a new public read. */
static RootValue *execution_value(AmaContextView view) {
    AmaPoolCell *cell = NULL;
    assert(ama_program_cell(view.program, view.control->root, &cell).code == AMA_OK);
    return ama_pool_payload(&view.world->types[1]->pool, cell);
}

/** Read a declared leaf's initialized native scalar under internal test-kernel authority. */
static NativeValue *execution_leaf(AmaContextView view, AmaHandle handle) {
    AmaPoolCell *cell = NULL;
    assert(ama_pool_lookup(&view.world->types[0]->pool, handle, false, &cell).code == AMA_OK);
    LeafValue *value = ama_pool_payload(&view.world->types[0]->pool, cell);
    return &value->native;
}

/** Simulate one root label's normal return/yield through actual runtime helpers.
 * No module is dispatched through a C function; this fixture exercises host control metadata only.
 */
static AmaContextOutcome return_execution(AmaContextExecution *execution, bool yielded, uint64_t selector) {
    AmaContextView view = execution_view(execution);
    AmaPoolCell *cell = NULL;
    assert(ama_program_cell(view.program, view.control->current, &cell).code == AMA_OK);
    if (yielded) { cell->metadata.service.resume = (AmaResumePosition){.valid = true, .selector = selector}; }
    uint64_t continuation = UINT64_MAX;
    AmaStatus status = yielded ? ama_program_yield(view.program, &view.control->current, &continuation)
        : ama_program_complete(view.program, &view.control->current, &continuation);
    assert(status.code == AMA_OK && continuation == 0);
    view.program->current = view.control->current;
    AmaContextOutcome outcome;
    assert(ama_context_execution_finish(execution, yielded ? AMA_HOST_YIELDED : AMA_HOST_COMPLETED,
        ama_status_success(), &outcome).code == AMA_OK);
    assert(execution->lease.context == NULL && execution->root.context == 0);
    return outcome;
}
#endif
