/**
 * @file
 * @brief Admit one selected context/root before any fallible entry preparation or Ama execution.
 *
 * [EMBED-03](../../../../spec/implementation/c-library.md#embed-03) keeps rejected
 * host requests effect-free. [CALL-08](../../../../spec/language/calls-and-results.md#call-08)
 * orders accepted interface initialization; resume bypasses that initialization.
 * [RT-11](../../../../spec/implementation/runtime.md#rt-11) constructs an
 * explicitly selected first root after context constants, without resetting its
 * freshly constructed interface a second time.
 */
#include "execution.h"
#include "../internal.h"

/** Validate host-side owner/mode shape without consulting mutable context state. */
static bool execution_request_valid(const AmaProgramInvocation *invocation, const AmaContextExecution *execution, bool leased) {
    return invocation != NULL && execution != NULL && (leased ? ama_context_leased(&execution->lease) != NULL
            : execution->lease.context == NULL && execution->lease.registry == NULL && execution->lease.identity == 0)
        && execution->root.context == 0 && ama_handle_is_empty(execution->root.local)
        && execution->selector == 0 && execution->status.code == AMA_OK
        && execution->status.message == NULL && execution->status.message_length == 0
        && invocation->target == NULL
        && (invocation->mode == AMA_ACCESS_ENTRY || invocation->mode == AMA_ACCESS_RESUME)
        && (invocation->input_count == 0 || invocation->inputs != NULL)
        && (invocation->mode != AMA_ACCESS_RESUME || invocation->input_count == 0);
}

/** Prepare an explicit host entry through the ordinary acquired-target path.
 * The selected root is also the temporary host source context; no C body runs.
 * Failure is an accepted execution failure, not an admission refusal.
 */
static AmaStatus execution_prepare(AmaContext *context, AmaPoolCell *cell,
    const AmaProgramInvocation *invocation) {
    AmaHandle root = context->control.root;
    cell->metadata.service.outgoing = root;
    cell->metadata.service.outgoing_previous = cell->metadata.service.activity;
    cell->metadata.service.activity = AMA_INSTANCE_ACQUIRED;
    AmaStatus status = ama_program_prepare(&context->program, root, invocation);
    if (status.code == AMA_OK) { cell->metadata.service.outgoing = (AmaHandle){0}; }
    return ama_status_checked(status);
}

/** Own the checked context lease through preparation and later generated label dispatch. */
AmaStatus ama_context_execution_begin_leased(AmaContextExecution *execution,
    AmaContextReference root, const AmaProgramInvocation *invocation) {
    if (!execution_request_valid(invocation, execution, true)) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    AmaStatus status = ama_status_success();
    AmaContext *context = ama_context_leased(&execution->lease);
    AmaObjectType *type = NULL;
    AmaPoolCell *cell = NULL;
    status = ama_context_resolve(&execution->lease, root, invocation->mode, &type, &cell);
    if (status.code == AMA_OK && (context->control.phase != AMA_RUNTIME_IDLE || context->world.operation_active)) {
        status = ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    if (status.code != AMA_OK) { return status; }
    /* Admission is complete: all later failures belong to this execution. */
    execution->root = root;
    execution->selector = invocation->mode == AMA_ACCESS_RESUME ? cell->metadata.service.resume.selector : 0;
    context->control = (AmaRuntimeMetadata){AMA_RUNTIME_EXECUTING, root.local, root.local};
    context->program.current = root.local;
    if (invocation->mode == AMA_ACCESS_ENTRY) { execution->status = execution_prepare(context, cell, invocation); }
    if (execution->status.code == AMA_OK) {
        cell->metadata.service.resume = (AmaResumePosition){0};
        cell->metadata.service.activity = AMA_INSTANCE_RUNNING;
        cell->metadata.service.return_link = (AmaReturnLink){.attached = true, .host = true};
    }
    return ama_status_success();
}

/** Select a first root explicitly, retaining one lease from private construction through label execution or accepted-failure cleanup. */
AmaStatus ama_context_execution_start(AmaContextRegistry *registry, uint64_t identity,
    uint64_t root_type, AmaContextExecution *execution) {
    const AmaProgramInvocation entry = {.mode = AMA_ACCESS_ENTRY};
    if (!execution_request_valid(&entry, execution, false)) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    AmaStatus status = ama_context_enter(registry, identity, &execution->lease);
    if (status.code != AMA_OK) { return status; }
    AmaContext *context = ama_context_leased(&execution->lease);
    if (context->control.phase != AMA_RUNTIME_IDLE || context->world.operation_active
        || context->world.failed || context->world.first_root != NULL
        || !ama_handle_is_empty(context->control.root)) {
        status = ama_status_make(AMA_INVALID_STATE, NULL, 0);
    } else if (ama_world_type(&context->world, root_type) == NULL) {
        status = ama_status_make(AMA_TYPE_MISMATCH, NULL, 0);
    }
    if (status.code != AMA_OK) {
        (void)ama_context_leave(&execution->lease);
        return status;
    }
    /* Construction itself initializes parameters/results exactly once. The
     * accepted owner also represents failed construction with no published root. */
    execution->root.context = identity;
    context->control.phase = AMA_RUNTIME_EXECUTING;
    execution->status = ama_object_create(&context->world, root_type, NULL, &execution->root.local);
    context->control.root = execution->root.local;
    context->control.current = execution->root.local;
    context->program.current = execution->root.local;
    if (execution->status.code == AMA_OK) {
        AmaPoolCell *cell = NULL;
        execution->status = ama_program_cell(&context->program, execution->root.local, &cell);
        if (execution->status.code == AMA_OK) {
            cell->metadata.service.activity = AMA_INSTANCE_RUNNING;
            cell->metadata.service.return_link = (AmaReturnLink){.attached = true, .host = true};
        }
    }
    return ama_status_success();
}

/** Preserve the ordinary owner-acquiring API through the same already-leased preparation path. */
AmaStatus ama_context_execution_begin(AmaContextRegistry *registry, uint64_t identity,
    AmaContextReference root, const AmaProgramInvocation *invocation, AmaContextExecution *execution) {
    if (!execution_request_valid(invocation, execution, false)) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    AmaStatus status = ama_context_enter(registry, identity, &execution->lease);
    if (status.code == AMA_OK) {
        status = ama_context_execution_begin_leased(execution, root, invocation);
        if (status.code != AMA_OK) { (void)ama_context_leave(&execution->lease); }
    }
    return status;
}
