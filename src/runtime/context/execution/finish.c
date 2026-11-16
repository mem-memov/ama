/**
 * @file
 * @brief Validate host return metadata or terminally clean one accepted context execution.
 *
 * [RT-14](../../../../spec/implementation/runtime.md#rt-14) requires explicit outcomes;
 * [LIFE-05](../../../../spec/implementation/native-interface.md#life-05) reports
 * active diagnostics before cleanup, preserving the original technical failure.
 */
#include "execution.h"
#include "../internal.h"

/** Require a healthy available selected root with no pending outgoing or host return link. Component contract: [RT-14](../../../../spec/implementation/runtime.md#rt-14), [LIFE-05](../../../../spec/implementation/native-interface.md#life-05). */
static AmaStatus execution_return_status(AmaContext *context, AmaHostOutcome outcome) {
    if (context->control.phase != AMA_RUNTIME_EXECUTING || context->world.failed
        || !ama_handle_equal(context->control.current, context->control.root)
        || (outcome != AMA_HOST_COMPLETED && outcome != AMA_HOST_YIELDED)) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    AmaPoolCell *cell = NULL;
    AmaStatus status = ama_program_cell(&context->program, context->control.root, &cell);
    if (status.code != AMA_OK) { return status; }
    status = ama_instance_access(&cell->metadata, AMA_ACCESS_READ);
    if (status.code != AMA_OK) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    AmaInstanceActivity expected = outcome == AMA_HOST_YIELDED ? AMA_INSTANCE_YIELDED : AMA_INSTANCE_READY;
    return cell->metadata.service.activity == expected && ama_handle_is_empty(cell->metadata.service.outgoing)
        ? ama_status_success() : ama_status_make(AMA_INVALID_STATE, NULL, 0);
}

/** Finalize valid execution state after native views close, retaining its lease for host results. Component contract: [RT-14](../../../../spec/implementation/runtime.md#rt-14), [LIFE-05](../../../../spec/implementation/native-interface.md#life-05). */
AmaStatus ama_context_execution_finish_retained(AmaContextExecution *execution, AmaHostOutcome outcome,
    AmaStatus status, AmaContextOutcome *output) {
    if (execution == NULL || output == NULL) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    AmaContext *context = ama_context_leased(&execution->lease);
    if (context == NULL || context->world.operation_active || execution->root.context != context->identity
        || !ama_handle_equal(execution->root.local, context->control.root)) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    context->program.current = context->control.current;
    AmaStatus primary = ama_cleanup_primary(context->status, execution->status);
    primary = ama_cleanup_primary(primary, ama_status_checked(status));
    if (primary.code == AMA_OK) { primary = execution_return_status(context, outcome); }
    AmaContextOutcome result = {.outcome = outcome, .root = execution->root, .status = primary};
    if (primary.code != AMA_OK) {
        result.outcome = AMA_HOST_FAILED;
        result.root = (AmaContextReference){0};
        result.status = ama_context_fail(&execution->lease, primary);
    } else { context->control.phase = AMA_RUNTIME_IDLE; }
    execution->root = (AmaContextReference){0};
    execution->selector = 0;
    execution->status = ama_status_success();
    *output = result;
    return ama_status_success();
}

/** Preserve the consuming API while allowing library hosts to retain exclusive result access. Component contract: [RT-14](../../../../spec/implementation/runtime.md#rt-14), [LIFE-05](../../../../spec/implementation/native-interface.md#life-05). */
AmaStatus ama_context_execution_finish(AmaContextExecution *execution, AmaHostOutcome outcome,
    AmaStatus status, AmaContextOutcome *output) {
    if (output == NULL) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    AmaContextOutcome result = {0};
    AmaStatus finished = ama_context_execution_finish_retained(execution, outcome, status, &result);
    if (finished.code != AMA_OK) { return finished; }
    AmaStatus released = ama_context_leave(&execution->lease);
    if (released.code == AMA_OK) { *output = result; }
    return released;
}
