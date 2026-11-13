/**
 * @file
 * @brief Type each adjacent assignment immediately after its first receiver is classified.
 * [TYPE-06](../../../../spec/language/declarations.md#type-06) visits the
 * receiver before its following source and forbids self or later-inferred type dependencies.
 */
#include "../internal.h"
bool inference_assignment(Inference *inference, const AstNode *node) {
    InferenceReceiver receiver;
    if (!inference_begin_receiver(inference, node->as.assignment.receiver, &receiver)) { return false; }
    inference->inferring = receiver.fresh ? receiver.syntax->name : (SourceSpan){0};
    InferenceValue source;
    bool typed = inference_source(inference, node->as.assignment.source, false, &source);
    inference->inferring = (SourceSpan){0};
    return typed && inference_finish_receiver(inference, &receiver, &source);
}
