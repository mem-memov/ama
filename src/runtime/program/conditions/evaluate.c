/** @file
 * @brief Convert a named caller value or a protected header result exactly once.
 * [PROTO-04](../../../../spec/implementation/native-interface.md#proto-04) and
 * [FLOW-03](../../../../spec/language/control-flow.md#flow-03) require the
 * selected type's explicit checked condition hook, never implicit truth.
 * [FLOW-04](../../../../spec/language/control-flow.md#flow-04) retains the
 * header owner during conversion while keeping index operands caller-local.
 */
#include "runtime/program/calls/call.h"
#include "runtime/program/values/selection/internal.h"

/** Borrow a readable scalar under one guard, perform its native conversion and
 * publish truth only after checked success. Owner and caller are explicit domains;
 * no Ama body executes or C activation survives this synchronous callback.
 */
static AmaStatus ama_condition_convert(AmaProgram *program, AmaHandle caller, AmaHandle owner,
    const AmaProgramAccess *access, bool *result) {
    if (program->world->operation_active || program->world->failed) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    program->world->operation_active = true;
    AmaProgramValue value = {0};
    AmaStatus status = ama_program_access_from(program, caller, owner, NULL, access, false, &value, NULL);
    const void *payload = NULL;
    if (status.code == AMA_OK) { status = ama_program_payload(program, &value, &payload); }
    const AmaProgramType *type = ama_program_type(program, value.type_id);
    if (status.code == AMA_OK) {
        bool condition = false;
        status = type == NULL || type->condition == NULL ? ama_status_make(AMA_TYPE_MISMATCH, NULL, 0)
            : ama_status_checked(type->condition(payload, &condition));
        if (status.code == AMA_OK) { *result = condition; }
    }
    program->world->operation_active = false;
    return status;
}

AmaStatus ama_program_condition(AmaProgram *program, AmaHandle caller, const AmaProgramAccess *access, bool *result) {
    return ama_condition_convert(program, caller, caller, access, result);
}

AmaStatus ama_program_condition_result(AmaProgram *program, AmaHandle caller, const AmaProgramAccess *access, bool *result) {
    AmaPoolCell *context = NULL;
    AmaStatus status = ama_program_cell(program, caller, &context);
    if (status.code != AMA_OK) { return status; }
    if (ama_handle_is_empty(context->metadata.service.outgoing)) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    return ama_condition_convert(program, caller, context->metadata.service.outgoing, access, result);
}
