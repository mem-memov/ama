/** @file
 * @brief Resolve direct method operands once without granting executed bodies access.
 * [LIST-10](../../../../../spec/language/lists.md#list-10) permits only exact
 * structural borrowing; [CALL-08](../../../../../spec/language/calls-and-results.md#call-08)
 * preserves the written source order independently of input ordinals.
 */
#include "internal.h"
#include "runtime/program/values/selection/internal.h"

AmaStatus ama_list_input_sources(AmaProgramListMethod *method,
    const AmaProgramListInputs *plan, size_t count, AmaListInputSources *sources) {
    AmaProgramListService *service = method->service;
    AmaProgram *program = service->program;
    if (plan->count > count) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    program->world->operation_active = true;
    AmaPoolCell *caller = NULL;
    AmaStatus status = ama_program_cell(program, service->caller, &caller);
    for (size_t index = 0; index < plan->count && status.code == AMA_OK; index++) {
        const AmaProgramInput *input = &plan->inputs[index];
        if (input->field >= count || sources->supplied[input->field] != NULL) {
            status = ama_status_make(AMA_INVALID_STATE, NULL, 0);
            break;
        }
        sources->supplied[input->field] = input;
        method->supplied_inputs |= 1u << input->field;
        caller->metadata.service.current_operation = input->operation;
        if (input->source.kind != AMA_SOURCE_LITERAL) {
            status = ama_program_evaluate_authorized(program, service->caller, NULL,
                &input->source, false, service, &sources->values[input->field]);
        }
    }
    program->world->operation_active = false;
    return status;
}
