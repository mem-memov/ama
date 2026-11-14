/**
 * @file
 * @brief Select and acquire one invocation target with explicit refusal provenance.
 *
 * [CALL-04](../../../../spec/language/calls-and-results.md#call-04)
 * Record only supported target acquisition refusals; protocol/technical failures cannot be guessed from their numeric code alone.
 * [LIST-04](../../../../spec/language/lists.md#list-04) admits materialized intrinsic values without synthetic owners.
 * [CALL-09](../../../../spec/language/calls-and-results.md#call-09) selects
 * children within protected parents without changing the caller operand domain.
 */
#include "call.h"
#include "runtime/program/values/selection/internal.h"

/** Resolve a local handle through the exact world type and public pool lookup; borrowed cell output is valid only after success. */
AmaStatus ama_program_cell(AmaProgram *program, AmaHandle handle, AmaPoolCell **cell) {
    AmaObjectType *type = ama_world_type(program->world, handle.type_id);
    return type == NULL ? ama_status_make(AMA_TYPE_MISMATCH, NULL, 0) : ama_pool_lookup(&type->pool, handle, false, cell);
}

/** Record a supported target refusal on the immediate caller and set the explicit refused flag only if recording succeeds. */
static AmaStatus ama_call_refusal(AmaPoolCell *caller, AmaStatus status, bool *refused) {
    AmaStatus recorded = ama_service_record_refusal(&caller->metadata.service, status);
    *refused = recorded.code == AMA_OK;
    return *refused ? status : recorded;
}

/** Validate selected identity/availability before changing the caller or target.
 * Parent protection does not authorize reentry through an ancestor reference.
 */
static AmaStatus ama_call_target(AmaProgram *program, AmaPoolCell *context,
    const AmaProgramValue *target, AmaAccessMode mode, AmaPoolCell **cell, bool *refused) {
    if (target->form != AMA_FIELD_OWNED && target->form != AMA_FIELD_REFERENCE) { return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0); }
    if (target->handle.generation == 0) { return ama_call_refusal(context, ama_status_make(AMA_EMPTY_REFERENCE, NULL, 0), refused); }
    AmaStatus status = ama_program_cell(program, target->handle, cell);
    if (status.code == AMA_OK) { status = ama_instance_access(&(*cell)->metadata, mode); }
    return status.code == AMA_OK ? status : ama_call_refusal(context, status, refused);
}

/** Select the target once under the world guard, preserve explicit refusal provenance and acquire only a valid scalar available instance. Store outgoing identity and previous activity on the caller. */
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
    AmaPoolCell *cell = NULL;
    status = ama_call_target(program, context, &target, invocation->mode, &cell, refused);
    if (status.code != AMA_OK) { return status; }
    context->metadata.service.outgoing = target.handle;
    context->metadata.service.outgoing_previous = cell->metadata.service.activity;
    cell->metadata.service.activity = AMA_INSTANCE_ACQUIRED;
    return ama_status_success();
}

/** Select once while the parent remains protected. Only accepted acquisition
 * activates the fixed caller slot; a refused child leaves no partial frame.
 * [CALL-09](../../../../spec/language/calls-and-results.md#call-09) keeps indices
 * and later argument/receiver evaluation in the initiating caller.
 */
AmaStatus ama_program_acquire_result(AmaProgram *program, AmaHandle caller,
    const AmaProgramInvocation *invocation, size_t offset, bool *refused) {
    *refused = false;
    if (program->world->operation_active || program->world->failed) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    AmaProgramFrame *frame = NULL;
    AmaPoolCell *context = NULL, *cell = NULL;
    AmaStatus status = ama_program_frame(program, caller, offset, &frame);
    if (status.code != AMA_OK) { return status; }
    status = ama_program_cell(program, caller, &context);
    if (status.code != AMA_OK) { return status; }
    if (frame->active || ama_handle_is_empty(context->metadata.service.outgoing)) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    AmaProgramValue target = {0};
    program->world->operation_active = true;
    status = ama_program_access_from(program, caller, context->metadata.service.outgoing,
        NULL, invocation->target, false, &target, refused);
    program->world->operation_active = false;
    if (status.code != AMA_OK) { return *refused ? ama_call_refusal(context, status, refused) : status; }
    status = ama_call_target(program, context, &target, invocation->mode, &cell, refused);
    if (status.code != AMA_OK) { return status; }
    *frame = (AmaProgramFrame){.active = true, .parent = context->metadata.service.outgoing,
        .previous = context->metadata.service.outgoing_previous};
    context->metadata.service.outgoing = target.handle;
    context->metadata.service.outgoing_previous = cell->metadata.service.activity;
    cell->metadata.service.activity = AMA_INSTANCE_ACQUIRED;
    return ama_status_success();
}

/** Intrinsic results already have their exact typed storage and need no module
 * selector. Park an optional outer module acquisition while this value executes.
 * [LIST-04](../../../../spec/language/lists.md#list-04) forbids fake method instances.
 */
AmaStatus ama_program_acquire_value(AmaProgram *program, AmaHandle caller,
    const AmaProgramValue *value, AmaAccessMode mode, size_t offset, bool *refused) {
    *refused = false;
    if (program->world->operation_active || program->world->failed) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    AmaProgramFrame *frame = NULL;
    AmaPoolCell *context = NULL, *target = NULL;
    AmaStatus status = ama_program_frame(program, caller, offset, &frame);
    if (status.code == AMA_OK) { status = ama_program_cell(program, caller, &context); }
    if (status.code != AMA_OK) { return status; }
    if (frame->active || value->constant != NULL || value->shared_list != NULL ||
        (value->handle.generation != 0 && value->type_id != value->handle.type_id)) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    status = ama_call_target(program, context, value, mode, &target, refused);
    if (status.code != AMA_OK) { return status; }
    *frame = (AmaProgramFrame){.active = true, .parent = context->metadata.service.outgoing,
        .previous = context->metadata.service.outgoing_previous};
    context->metadata.service.outgoing = value->handle;
    context->metadata.service.outgoing_previous = target->metadata.service.activity;
    target->metadata.service.activity = AMA_INSTANCE_ACQUIRED;
    return ama_status_success();
}

/** After protected result processing, restore target availability saved by the caller and clear the outgoing handle. Require a valid outstanding acquisition. */
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
