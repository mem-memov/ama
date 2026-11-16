/**
 * @file
 * @brief Connect and return between stable instances using explicit caller links.
 *
 * [CONT-01](../../../../spec/language/continuations.md#cont-01)
 * Yield returns to the immediate caller, retains a separate resume position and keeps results acquired for caller-side processing.
 */
#include "call.h"

/** Require an acquired outgoing target, attach the immediate caller/return selector/site, mark caller Waiting and target Running, and change current only after checks. Component contract: [CONT-01](../../../../spec/language/continuations.md#cont-01). */
AmaStatus ama_program_connect(AmaProgram *program, AmaHandle *current, uint64_t continuation, uint64_t site) {
    AmaPoolCell *caller = NULL, *target = NULL;
    AmaStatus status = ama_program_cell(program, *current, &caller);
    if (status.code != AMA_OK) { return status; }
    AmaHandle handle = caller->metadata.service.outgoing;
    status = ama_program_cell(program, handle, &target);
    if (status.code != AMA_OK) { return status; }
    if (target->metadata.service.activity != AMA_INSTANCE_ACQUIRED) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    target->metadata.service.return_link = (AmaReturnLink){.attached = true, .caller = *current,
        .continuation = continuation, .call_site = site};
    target->metadata.service.activity = AMA_INSTANCE_RUNNING;
    caller->metadata.service.activity = AMA_INSTANCE_WAITING;
    *current = handle;
    return ama_status_success();
}

/** Validate current Running state, no nested outgoing call and a matching return connection before mutation. Host returns become available directly; ordinary returns retain target acquisition for caller result processing. Component contract: [CONT-01](../../../../spec/language/continuations.md#cont-01). */
static AmaStatus ama_program_return(AmaProgram *program, AmaHandle *current, uint64_t *continuation, bool yielded) {
    AmaPoolCell *cell = NULL, *caller = NULL;
    AmaStatus status = ama_program_cell(program, *current, &cell);
    if (status.code != AMA_OK) { return status; }
    if (cell->metadata.service.activity != AMA_INSTANCE_RUNNING ||
        !ama_handle_is_empty(cell->metadata.service.outgoing)) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    AmaReturnLink link = cell->metadata.service.return_link;
    if (!ama_return_link_is_valid(link) || !link.attached) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    if (yielded && (!cell->metadata.service.resume.valid || cell->metadata.service.resume.selector == 0)) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    if (!link.host) {
        status = ama_program_cell(program, link.caller, &caller);
        if (status.code != AMA_OK) { return status; }
        if (caller->metadata.service.activity != AMA_INSTANCE_WAITING ||
            !ama_handle_equal(caller->metadata.service.outgoing, *current)) {
            return ama_status_make(AMA_INVALID_STATE, NULL, 0);
        }
    }
    AmaInstanceActivity available = yielded ? AMA_INSTANCE_YIELDED : AMA_INSTANCE_READY;
    cell->metadata.service.return_link = (AmaReturnLink){0};
    if (!yielded) { cell->metadata.service.resume = (AmaResumePosition){0}; }
    if (link.host) { cell->metadata.service.activity = available; *continuation = 0; return ama_status_success(); }
    cell->metadata.service.activity = AMA_INSTANCE_ACQUIRED;
    caller->metadata.service.activity = AMA_INSTANCE_RUNNING;
    caller->metadata.service.outgoing_previous = available;
    *current = link.caller;
    *continuation = link.continuation;
    return ama_status_success();
}

/** Return normally to the immediate caller or host and clear the target saved resume position; output continuation selects the caller label or zero for host. Component contract: [CONT-01](../../../../spec/language/continuations.md#cont-01). */
AmaStatus ama_program_complete(AmaProgram *program, AmaHandle *current, uint64_t *continuation) {
    return ama_program_return(program, current, continuation, false);
}

/** Require an explicit nonzero saved resume selector and return to the immediate caller or host while preserving it; no active caller chain is copied. Component contract: [CONT-01](../../../../spec/language/continuations.md#cont-01). */
AmaStatus ama_program_yield(AmaProgram *program, AmaHandle *current, uint64_t *continuation) {
    return ama_program_return(program, current, continuation, true);
}
