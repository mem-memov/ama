/** @file
 * @brief Execute typed remove/clear after complete owning-subtree preflight.
 * [LIST-06](../../../../../spec/language/lists.md#list-06) protects all removed
 * resources before destruction; [LIST-10](../../../../../spec/language/lists.md#list-10)
 * keeps destruction failure technical and parent authority active through cleanup.
 */
#include "deletion.h"
#include "runtime/lists/list.h"

/** Remove the already converted range without releasing the service parent. Component contract: [LIST-06](../../../../../spec/language/lists.md#list-06), [LIST-10](../../../../../spec/language/lists.md#list-10). */
static AmaStatus ama_list_deletion_commit(AmaProgramListMethod *method,
    const AmaProgramListInputs *plan, size_t position, size_t count, bool *refused) {
    AmaProgramListService *service = method->service;
    AmaProgram *program = service->program;
    AmaPoolCell *caller = NULL;
    AmaStatus status = ama_program_cell(program, service->caller, &caller);
    if (status.code != AMA_OK) { return status; }
    caller->metadata.service.current_operation = plan->operation;
    AmaObjectType *owner = ama_world_type(program->world, service->owner.type_id);
    AmaListStorage storage = {program->world, service->owner, service->list,
        owner->fields[service->field].type_id, owner->fields[service->field].kind == AMA_FIELD_OWNING_LIST};
    status = ama_list_storage_remove_checked(&storage, position, count, refused);
    return ama_program_list_record_refusal(method, status, refused);
}

AmaStatus ama_program_list_remove(AmaProgramListService *service, AmaProgramListMethod *method,
    const AmaProgramListInputs *plan, bool *refused) {
    *refused = false;
    AmaStatus status = ama_program_list_method_begin(service, method);
    if (status.code != AMA_OK) { return status; }
    static const AmaFieldKind inputs[] = {AMA_FIELD_OWNED};
    status = ama_program_list_prepare_inputs(method, plan, inputs, 1);
    if (status.code != AMA_OK) { return status; }
    size_t position = SIZE_MAX;
    status = ama_program_list_index(method, plan->integer_type, 0, &position, refused);
    return status.code == AMA_OK ? ama_list_deletion_commit(method, plan, position, 1, refused)
        : ama_program_list_record_refusal(method, status, refused);
}

AmaStatus ama_program_list_clear(AmaProgramListService *service, AmaProgramListMethod *method,
    const AmaProgramListInputs *plan, bool *refused) {
    *refused = false;
    AmaStatus status = ama_program_list_method_begin(service, method);
    if (status.code != AMA_OK) { return status; }
    status = ama_program_list_prepare_inputs(method, plan, NULL, 0);
    return status.code == AMA_OK ? ama_list_deletion_commit(method, plan, 0, service->list->count, refused) : status;
}
