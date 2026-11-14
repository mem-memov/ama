/** @file
 * @brief Convert prepared index values and record only explicit preflight refusals.
 * [LIST-10](../../../../../spec/language/lists.md#list-10) reserves invalid-index
 * conversion while keeping other native errors technical and outputs unconsumed.
 */
#include "runtime/program/lists/inputs/inputs.h"

AmaStatus ama_program_list_index(AmaProgramListMethod *method, uint64_t integer_type,
    size_t input, size_t *position, bool *refused) {
    *refused = false;
    if (!method->active || input >= 2 || input >= method->input_count || (method->owned_inputs & (1u << input)) == 0) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    AmaProgram *program = method->service->program;
    const AmaProgramType *type = ama_program_type(program, integer_type);
    if (type == NULL || type->index == NULL) { return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0); }
    AmaProgramValue value = {.type_id = integer_type, .form = AMA_FIELD_OWNED, .handle = method->inputs[input]};
    const void *payload = NULL;
    program->world->operation_active = true;
    AmaStatus status = ama_program_payload(program, &value, &payload);
    size_t converted = SIZE_MAX;
    if (status.code == AMA_OK) { status = ama_status_checked(type->index(payload, &converted)); }
    program->world->operation_active = false;
    *refused = status.code == AMA_INVALID_INDEX;
    if (status.code == AMA_OK) { *position = converted; }
    return status;
}

AmaStatus ama_program_list_record_refusal(AmaProgramListMethod *method, AmaStatus status, bool *refused) {
    if (!*refused) { return status; }
    AmaPoolCell *caller = NULL;
    AmaStatus recorded = ama_program_cell(method->service->program, method->service->caller, &caller);
    if (recorded.code == AMA_OK) { recorded = ama_service_record_refusal(&caller->metadata.service, status); }
    if (recorded.code != AMA_OK) { *refused = false; return recorded; }
    return status;
}
