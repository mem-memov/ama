#include "call.h"

AmaStatus ama_program_cell(AmaProgram *program, AmaHandle handle, AmaPoolCell **cell) {
    AmaObjectType *type = ama_world_type(program->world, handle.type_id);
    return type == NULL ? ama_status_make(AMA_TYPE_MISMATCH, NULL, 0) : ama_pool_lookup(&type->pool, handle, false, cell);
}

static AmaStatus ama_call_refusal(AmaPoolCell *caller, AmaStatus status, bool *refused) {
    AmaStatus recorded = ama_service_record_refusal(&caller->metadata.service, status);
    *refused = recorded.code == AMA_OK;
    return *refused ? status : recorded;
}

AmaStatus ama_program_acquire(AmaProgram *program, AmaHandle caller, const AmaProgramInvocation *invocation, bool *refused) {
    *refused = false;
    if (program->world->operation_active || program->world->failed) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    AmaPoolCell *context = NULL;
    AmaStatus status = ama_program_cell(program, caller, &context);
    if (status.code != AMA_OK || context->metadata.service.outgoing.generation != 0) {
        return status.code != AMA_OK ? status : ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    AmaProgramValue target = {0};
    program->world->operation_active = true;
    status = ama_program_select_target(program, caller, invocation->target, &target, refused);
    program->world->operation_active = false;
    if (status.code != AMA_OK) { return *refused ? ama_call_refusal(context,status,refused) : status; }
    if (target.form != AMA_FIELD_OWNED && target.form != AMA_FIELD_REFERENCE) { return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0); }
    if (target.handle.generation == 0) { return ama_call_refusal(context,ama_status_make(AMA_EMPTY_REFERENCE, NULL, 0),refused); }
    AmaPoolCell *cell = NULL;
    status = ama_program_cell(program, target.handle, &cell);
    if (status.code == AMA_OK) { status = ama_instance_access(&cell->metadata, invocation->mode); }
    if (status.code != AMA_OK) { return ama_call_refusal(context,status,refused); }
    context->metadata.service.outgoing = target.handle;
    context->metadata.service.outgoing_previous = cell->metadata.service.activity;
    cell->metadata.service.activity = AMA_INSTANCE_ACQUIRED;
    return ama_status_success();
}

AmaStatus ama_program_release(AmaProgram *program, AmaHandle caller) {
    AmaPoolCell *context = NULL, *target = NULL;
    AmaStatus status = ama_program_cell(program, caller, &context);
    if (status.code != AMA_OK) { return status; }
    status = ama_program_cell(program, context->metadata.service.outgoing, &target);
    if (status.code != AMA_OK) { return status; }
    target->metadata.service.activity = context->metadata.service.outgoing_previous;
    context->metadata.service.outgoing = (AmaHandle){0};
    return ama_status_success();
}
