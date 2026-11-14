/** @file
 * @brief Classify pure preflight refusal and publish append/insert effects once.
 * [LIST-10](../../../../../spec/language/lists.md#list-10) admits only reserved
 * invalid-index and checked availability refusals; native construction failures
 * remain technical even when their code equals an acquisition refusal.
 */
#include "internal.h"
#include "runtime/lists/list.h"

/** Invoke the prepared integer's declared read-only conversion exactly once. */
static AmaStatus ama_list_insertion_index(AmaProgramListMethod *method,
    const AmaProgramListInputs *plan, size_t *position, bool *refused) {
    AmaProgram *program = method->service->program;
    const AmaProgramType *type = ama_program_type(program, plan->integer_type);
    if (type == NULL || type->index == NULL) { return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0); }
    AmaProgramValue value = {.type_id = plan->integer_type, .form = AMA_FIELD_OWNED, .handle = method->inputs[0]};
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

/** Record only an explicitly classified preflight refusal, never a native code guess. */
static AmaStatus ama_list_insertion_refusal(AmaProgramListMethod *method, AmaStatus status, bool *refused) {
    if (!*refused) { return status; }
    AmaPoolCell *caller = NULL;
    AmaStatus recorded = ama_program_cell(method->service->program, method->service->caller, &caller);
    if (recorded.code == AMA_OK) { recorded = ama_service_record_refusal(&caller->metadata.service, status); }
    if (recorded.code != AMA_OK) { *refused = false; return recorded; }
    return status;
}

AmaStatus ama_list_insertion_commit(AmaProgramListMethod *method, const AmaProgramListInputs *plan,
    const AmaListInsertionInputs *inputs, bool *refused) {
    AmaProgramListService *service = method->service;
    AmaProgram *program = service->program;
    AmaPoolCell *caller = NULL;
    AmaStatus status = ama_program_cell(program, service->caller, &caller);
    if (status.code != AMA_OK) { return status; }
    caller->metadata.service.current_operation = plan->operation;
    size_t position = service->list->count;
    if (inputs->positioned) { status = ama_list_insertion_index(method, plan, &position, refused); }
    if (status.code != AMA_OK) { return ama_list_insertion_refusal(method, status, refused); }
    AmaObjectType *owner = ama_world_type(program->world, service->owner.type_id);
    bool owning = owner->fields[service->field].kind == AMA_FIELD_OWNING_LIST;
    AmaListStorage storage = {program->world, service->owner, service->list,
        owner->fields[service->field].type_id, owning};
    status = ama_list_storage_insert_checked(&storage, position, method->inputs[inputs->source_position],
        inputs->supplied[inputs->source_position] != NULL && owning, refused);
    if (status.code != AMA_OK) { return ama_list_insertion_refusal(method, status, refused); }
    AmaHandle result = *ama_handle_list_element(service->list, position);
    return ama_program_list_reference_result(method, result);
}
