/** @file
 * @brief Classify pure preflight refusal and publish append/insert effects once.
 * [LIST-10](../../../../../spec/language/lists.md#list-10) admits only reserved
 * invalid-index and checked availability refusals; native construction failures
 * remain technical even when their code equals an acquisition refusal.
 */
#include "internal.h"
#include "runtime/lists/list.h"

AmaStatus ama_list_insertion_commit(AmaProgramListMethod *method, const AmaProgramListInputs *plan,
    bool positioned, bool *refused) {
    AmaProgramListService *service = method->service;
    AmaProgram *program = service->program;
    AmaPoolCell *caller = NULL;
    AmaStatus status = ama_program_cell(program, service->caller, &caller);
    if (status.code != AMA_OK) { return status; }
    caller->metadata.service.current_operation = plan->operation;
    size_t position = service->list->count;
    if (positioned) { status = ama_program_list_index(method, plan->integer_type, 0, &position, refused); }
    if (status.code != AMA_OK) { return ama_program_list_record_refusal(method, status, refused); }
    AmaObjectType *owner = ama_world_type(program->world, service->owner.type_id);
    bool owning = owner->fields[service->field].kind == AMA_FIELD_OWNING_LIST;
    AmaListStorage storage = {program->world, service->owner, service->list,
        owner->fields[service->field].type_id, owning};
    size_t source_position = positioned ? 1 : 0;
    status = ama_list_storage_insert_checked(&storage, position, method->inputs[source_position],
        (method->supplied_inputs & (1u << source_position)) != 0 && owning, refused);
    if (status.code != AMA_OK) { return ama_program_list_record_refusal(method, status, refused); }
    AmaHandle result = *ama_handle_list_element(service->list, position);
    return ama_program_list_reference_result(method, result);
}
