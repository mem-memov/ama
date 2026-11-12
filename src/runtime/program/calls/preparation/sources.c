/**
 * @file
 * @brief Select every supplied input exactly once before source acquisition.
 *
 * [CALL-08](../../../../../spec/language/calls-and-results.md#call-08)
 * requires binding-text source order before any interface field reset.
 */
#include "internal.h"

AmaStatus ama_call_select_sources(AmaProgram *program, AmaHandle caller, const AmaProgramInvocation *invocation,
    AmaPreparedInput *inputs) {
    AmaPoolCell *context = NULL;
    AmaStatus status = ama_program_cell(program, caller, &context);
    if (status.code != AMA_OK) { return status; }
    for (size_t index = 0; index < invocation->input_count; index++) {
        inputs[index].input = &invocation->inputs[index];
        const AmaProgramSource *source = &inputs[index].input->source;
        context->metadata.service.current_operation = inputs[index].input->operation;
        if (source->kind == AMA_SOURCE_LITERAL) { continue; }
        status = ama_program_evaluate(program, caller, NULL, source, false, &inputs[index].value);
        if (status.code != AMA_OK) { return status; }
    }
    return status;
}
