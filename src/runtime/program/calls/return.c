#include "call.h"

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
    target->metadata.service.resume = (AmaResumePosition){0};
    target->metadata.service.activity = AMA_INSTANCE_RUNNING;
    caller->metadata.service.activity = AMA_INSTANCE_WAITING;
    *current = handle;
    return ama_status_success();
}

AmaStatus ama_program_complete(AmaProgram *program, AmaHandle *current, uint64_t *continuation) {
    AmaPoolCell *cell = NULL, *caller = NULL;
    AmaStatus status = ama_program_cell(program, *current, &cell);
    if (status.code != AMA_OK) { return status; }
    AmaReturnLink link = cell->metadata.service.return_link;
    if (!ama_return_link_is_valid(link) || !link.attached) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    cell->metadata.service.return_link = (AmaReturnLink){0};
    cell->metadata.service.resume = (AmaResumePosition){0};
    if (link.host) { cell->metadata.service.activity = AMA_INSTANCE_READY; *continuation = 0; return ama_status_success(); }
    status = ama_program_cell(program, link.caller, &caller);
    if (status.code != AMA_OK) { return status; }
    cell->metadata.service.activity = AMA_INSTANCE_ACQUIRED;
    caller->metadata.service.activity = AMA_INSTANCE_RUNNING;
    caller->metadata.service.outgoing_previous = AMA_INSTANCE_READY;
    *current = link.caller;
    *continuation = link.continuation;
    return ama_status_success();
}
