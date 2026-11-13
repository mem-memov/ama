/**
 * @file
 * @brief Visit each binding at its written position and type new receivers from the selected declared result.
 * [TYPE-06](../../../../spec/language/declarations.md#type-06) excludes labels
 * and placeholders from module-name occurrences; source/index operands remain caller-side uses.
 */
#include "../internal.h"
bool inference_binding(Inference *inference, const InferenceEvent *event) {
    const AstArgument *argument = event->argument;
    if (argument->omitted) { return true; }
    if (argument->kind == AST_INPUT) {
        InferenceValue source;
        return inference_source(inference, argument->source, false, &source);
    }
    InferenceOperation *operation = inference_find_operation(inference, event->node);
    if (operation == NULL) {
        return semantic_fail(inference->program, event->site, "The operation target has not been typed", NULL);
    }
    InferenceValue result;
    return inference_result(inference, operation, argument->label, argument->position, argument->span, &result)
        && inference_receiver(inference, argument->receiver, &result);
}
