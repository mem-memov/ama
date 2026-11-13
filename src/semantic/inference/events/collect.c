/**
 * @file
 * @brief Create inference events for actual written operands rather than grouped AST argument storage.
 * [TYPE-06](../../../../spec/language/declarations.md#type-06) requires a single
 * source-byte order even when ordinary bindings interleave selected subtrees.
 */
#include <stdint.h>
#include "../internal.h"
/** Append one checked finite event before publication; all syntax remains project-owned. */
static bool append(Inference *inference, InferenceEvent event) {
    if (inference->event_count == SIZE_MAX) { return semantic_memory_failure(inference->program); }
    InferenceEvent *owned = semantic_allocate(inference->program, sizeof(*owned));
    if (owned == NULL) { return false; }
    *owned = event;
    if (inference->last == NULL) { inference->first = owned; }
    else { inference->last->next = owned; }
    inference->last = owned;
    inference->event_count++;
    return true;
}
/** Schedule one node's own source events; structural children are supplied once by the semantic walker. */
static bool node_events(SemanticProgram *program, SemanticType *type,
    const AstNode *node, size_t depth, const AstNode *loop, void *context) {
    (void)program;
    (void)depth;
    (void)loop;
    Inference *inference = context;
    InferenceEvent event = {.node = node, .site = node->span};
    if (node->kind == AST_DECLARATION) {
        event.kind = INFER_DECLARATION;
        if (!append(inference, event)) { return false; }
        SemanticSymbol *symbol = semantic_symbol(type, node->as.declaration.name->name);
        for (const AstInitializer *item = node->as.declaration.first; item != NULL; item = item->next) {
            if (!append(inference, (InferenceEvent){.kind = INFER_INITIALIZER,
                    .node = node, .site = item->source->span, .source = item->source,
                    .initializing = symbol})) { return false; }
        }
        return true;
    }
    if (node->kind == AST_ASSIGNMENT) { event.kind = INFER_ASSIGNMENT; }
    else if (node->kind == AST_CALL) {
        event.kind = INFER_CALL;
        if (!append(inference, event)) { return false; }
        for (const AstArgument *argument = node->as.call.first; argument != NULL; argument = argument->next) {
            if (!append(inference, (InferenceEvent){.kind = INFER_BINDING, .node = node,
                    .argument = argument, .site = argument->span})) { return false; }
        }
        return true;
    } else if (node->kind == AST_BRANCH || node->kind == AST_LOOP) {
        event.kind = INFER_CONDITION;
        event.site = node->as.control.condition->span;
    } else if (node->kind == AST_FOREACH) { event.kind = INFER_FOREACH; }
    else { return true; }
    return append(inference, event);
}
bool inference_collect(Inference *inference) {
    const AstModule *syntax = &inference->type->module->syntax;
    AstBlock blocks[] = {syntax->constants, syntax->parameters, syntax->results, syntax->body};
    for (size_t index = 0; index < sizeof(blocks) / sizeof(blocks[0]); index++) {
        if (!semantic_walk(inference->program, inference->type, blocks[index], node_events, inference)) { return false; }
    }
    return true;
}
