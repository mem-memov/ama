/** @file
 * @brief Install fixed scalar parameter forms in declaration order after selection.
 * [LIST-04](../../../../../spec/language/lists.md#list-04) gives each method an
 * exact signature, never irrelevant parameters selected by a runtime action value.
 */
#include "internal.h"

AmaStatus ama_program_list_prepare_inputs(AmaProgramListMethod *method,
    const AmaProgramListInputs *plan, const AmaFieldKind *forms, size_t count) {
    if (!method->active || method->service == NULL || method->service->method != method || count > 2
        || method->input_count != 0 || method->owned_inputs != 0) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    AmaProgramListService *service = method->service;
    AmaProgram *program = service->program;
    if (program->world->operation_active || program->world->failed) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    for (size_t index = 0; index < count; index++) {
        if (forms[index] != AMA_FIELD_OWNED && forms[index] != AMA_FIELD_REFERENCE) {
            return ama_status_make(AMA_INVALID_STATE, NULL, 0);
        }
    }
    AmaListInputSources sources = {0};
    AmaStatus status = ama_list_input_sources(method, plan, count, &sources);
    if (status.code != AMA_OK) { return status; }
    method->input_count = count;
    AmaObjectType *owner = ama_world_type(program->world, service->owner.type_id);
    uint64_t element_type = owner->fields[service->field].type_id;
    for (size_t position = 0; position < count; position++) {
        if (forms[position] == AMA_FIELD_OWNED) {
            status = ama_list_input_integer(method, plan, &sources, position);
            if (status.code != AMA_OK) { return status; }
        } else if (sources.supplied[position] != NULL) {
            const AmaProgramValue *value = &sources.values[position];
            if (sources.supplied[position]->source.kind == AMA_SOURCE_LITERAL || value->form != AMA_FIELD_REFERENCE
                || value->type_id != element_type || (value->handle.generation != 0 && value->handle.type_id != element_type)) {
                return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0);
            }
            method->inputs[position] = value->handle;
        }
    }
    return ama_status_success();
}
