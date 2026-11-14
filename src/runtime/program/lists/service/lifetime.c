/** @file
 * @brief Acquire one list while borrowing the caller's exact owner protection.
 * [LIST-10](../../../../../spec/language/lists.md#list-10) keeps the grant live
 * through siblings and output subtrees, then releases it before a local handler.
 */
#include "runtime/program/lists/service.h"
#include "runtime/program/calls/call.h"

AmaStatus ama_program_list_open(AmaProgram *program, AmaHandle caller, AmaHandle owner,
    size_t field_index, AmaProgramListService *service, bool *refused) {
    *refused = false;
    if (program->world->operation_active || program->world->failed || service->active ||
        !ama_handle_equal(program->current, caller)) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    AmaPoolCell *context = NULL, *parent = NULL;
    AmaStatus status = ama_program_cell(program, caller, &context);
    if (status.code == AMA_OK) { status = ama_program_cell(program, owner, &parent); }
    if (status.code != AMA_OK) { return status; }
    bool self = ama_handle_equal(caller, owner);
    if (context->metadata.service.activity != AMA_INSTANCE_RUNNING ||
        (!self && (!ama_handle_equal(context->metadata.service.outgoing, owner)
            || parent->metadata.service.activity != AMA_INSTANCE_ACQUIRED))) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    AmaObjectType *type = ama_world_type(program->world, owner.type_id);
    if (field_index >= type->field_count) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    const AmaObjectField *field = &type->fields[field_index];
    if (field->kind != AMA_FIELD_OWNING_LIST && field->kind != AMA_FIELD_REFERENCE_LIST) {
        return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0);
    }
    AmaHandleList *list = (void *)((unsigned char *)ama_pool_payload(&type->pool, parent) + field->offset);
    if (!ama_handle_equal(list->owner, owner) || list->field != field_index || list->incarnation == 0) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    if (list->acquisition != NULL) {
        status = ama_status_make(AMA_BUSY, NULL, 0);
        AmaStatus recorded = ama_service_record_refusal(&context->metadata.service, status);
        *refused = recorded.code == AMA_OK;
        return *refused ? status : recorded;
    }
    *service = (AmaProgramListService){.program = program, .caller = caller, .owner = owner,
        .field = field_index, .incarnation = list->incarnation, .list = list, .active = true};
    list->acquisition = service;
    return ama_status_success();
}

AmaStatus ama_program_list_close(AmaProgramListService *service) {
    AmaStatus status = ama_program_list_validate(service);
    if (status.code != AMA_OK) { return status; }
    if (service->method != NULL || service->program->world->operation_active) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    service->list->acquisition = NULL;
    *service = (AmaProgramListService){0};
    return ama_status_success();
}
