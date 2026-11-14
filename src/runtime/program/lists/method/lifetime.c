/** @file
 * @brief Own one method's typed result through its entire output subtree.
 * [LIST-04](../../../../../spec/language/lists.md#list-04) requires one explicit
 * result lifetime, not a hidden method instance or retained result state.
 */
#include "runtime/program/lists/service.h"
#include "runtime/lifecycle/errors.h"

AmaStatus ama_program_list_method_begin(AmaProgramListService *service, AmaProgramListMethod *method) {
    AmaStatus status = ama_program_list_validate(service);
    if (status.code != AMA_OK) { return status; }
    if (service->program->world->failed || service->program->world->operation_active ||
        !ama_handle_equal(service->program->current, service->caller) || service->method != NULL || method->active) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    *method = (AmaProgramListMethod){.service = service, .active = true};
    service->method = method;
    return ama_status_success();
}

AmaStatus ama_program_list_method_end(AmaProgramListMethod *method) {
    if (!method->active || method->service == NULL || method->service->method != method) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    AmaStatus status = ama_program_list_validate(method->service);
    if (status.code != AMA_OK) { return status; }
    AmaProgram *program = method->service->program;
    bool owned = method->owns_result;
    AmaHandle result = method->value.handle;
    AmaHandle inputs[2] = {method->inputs[0], method->inputs[1]};
    size_t input_count = method->input_count;
    unsigned owned_inputs = method->owned_inputs;
    if (input_count > 2) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    if (program->world->operation_active) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    AmaProgramListService *service = method->service;
    method->active = method->owns_result = method->has_result = false;
    method->owned_inputs = 0;
    /* Consume before destruction, but retain the parent's method link until
     * cleanup returns. No callback can close the service between those events. */
    status = owned ? ama_object_destroy(program->world, result) : ama_status_success();
    while (input_count != 0) {
        size_t index = --input_count;
        if ((owned_inputs & (1u << index)) != 0) {
            status = ama_cleanup_primary(status, ama_object_destroy(program->world, inputs[index]));
        }
    }
    service->method = NULL;
    *method = (AmaProgramListMethod){0};
    return status;
}
