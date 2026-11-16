/** @file
 * @brief Materialize ordinary typed method values after successful preflight.
 * [LIST-04](../../../../../spec/language/lists.md#list-04) gives owned results
 * stable explicit roots and references their ordinary nonowning copy semantics.
 */
#include "runtime/program/lists/service.h"

/** Validate the exact method credit and reject a second result construction. Component contract: [LIST-04](../../../../../spec/language/lists.md#list-04). */
static AmaStatus ama_list_result_ready(const AmaProgramListMethod *method) {
    if (!method->active || method->service == NULL || method->service->method != method || method->has_result) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    AmaProgram *program = method->service->program;
    if (program->world->failed || program->world->operation_active ||
        !ama_handle_equal(program->current, method->service->caller)) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    return ama_program_list_validate(method->service);
}

AmaStatus ama_program_list_reference_result(AmaProgramListMethod *method, AmaHandle target) {
    AmaStatus status = ama_list_result_ready(method);
    if (status.code != AMA_OK) { return status; }
    AmaProgramListService *service = method->service;
    AmaObjectType *owner = ama_world_type(service->program->world, service->owner.type_id);
    uint64_t type = owner->fields[service->field].type_id;
    if (target.generation != 0 && target.type_id != type) { return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0); }
    method->value = (AmaProgramValue){.type_id = type, .form = AMA_FIELD_REFERENCE, .handle = target};
    method->has_result = true;
    return ama_status_success();
}

AmaStatus ama_program_list_owned_result(AmaProgramListMethod *method, uint64_t type, const AmaRecipe *recipe) {
    AmaStatus status = ama_list_result_ready(method);
    if (status.code != AMA_OK) { return status; }
    AmaHandle result = {0};
    status = ama_object_create(method->service->program->world, type, recipe, &result);
    if (status.code != AMA_OK) { return status; }
    method->value = (AmaProgramValue){.type_id = type, .form = AMA_FIELD_OWNED, .handle = result};
    method->has_result = method->owns_result = true;
    return ama_status_success();
}
