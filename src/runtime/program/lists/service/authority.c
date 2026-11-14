/** @file
 * @brief Validate the exact linear collection grant without caller-name privileges.
 * [LIST-10](../../../../../spec/language/lists.md#list-10) permits only direct
 * structural helpers to borrow the parent's grant, never ordinary executed bodies.
 */
#include "runtime/program/lists/service.h"
#include "runtime/program/calls/call.h"

AmaStatus ama_program_list_validate(const AmaProgramListService *service) {
    if (service == NULL || !service->active || service->program == NULL) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    AmaProgram *program = service->program;
    AmaObjectType *type = ama_world_type(program->world, service->owner.type_id);
    AmaPoolCell *owner = NULL;
    AmaStatus status = ama_program_cell(program, service->owner, &owner);
    if (status.code != AMA_OK) { return status; }
    if (service->field >= type->field_count) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    const AmaObjectField *field = &type->fields[service->field];
    if (field->kind != AMA_FIELD_OWNING_LIST && field->kind != AMA_FIELD_REFERENCE_LIST) {
        return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0);
    }
    AmaHandleList *list = (void *)((unsigned char *)ama_pool_payload(&type->pool, owner) + field->offset);
    bool self = ama_handle_equal(service->owner, service->caller);
    AmaInstanceActivity activity = owner->metadata.service.activity;
    if (list != service->list || list->acquisition != service || list->incarnation != service->incarnation ||
        !ama_handle_equal(list->owner, service->owner) || list->field != service->field ||
        (self ? activity != AMA_INSTANCE_RUNNING && activity != AMA_INSTANCE_WAITING
              : activity != AMA_INSTANCE_ACQUIRED)) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    return ama_status_success();
}

AmaStatus ama_program_list_authorize(const AmaProgramListService *service, AmaProgram *program,
    AmaHandle owner, size_t field, const AmaHandleList *list) {
    if (service == NULL || service->program != program || !ama_handle_equal(service->owner, owner)
        || service->field != field || service->list != list || !ama_handle_equal(program->current, service->caller)) {
        return ama_status_make(AMA_BUSY, NULL, 0);
    }
    AmaStatus status = ama_program_list_validate(service);
    return status.code == AMA_OK ? status : ama_status_make(AMA_BUSY, NULL, 0);
}
