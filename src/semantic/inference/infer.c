/**
 * @file
 * @brief Apply one exact source-ordered declaration/type pass before capability and transfer validation.
 * [TYPE-06](../../../spec/language/declarations.md#type-06) requires immediate
 * inferred types; [NAME-06](../../../spec/language/declarations.md#name-06)
 * derives local scope from written foreach bodies, independently of runtime contents.
 */
#include <assert.h>
#include "internal.h"
/** Resolve a condition's caller use or exact declared terminal without replaying any source operation. */
static bool condition(Inference *inference, const AstNode *node) {
    InferenceValue value;
    if (node->as.control.command == NULL) {
        return inference_access(inference, node->as.control.condition, false, &value);
    }
    InferenceOperation *owner = inference_find_operation(inference, node->as.control.condition_owner);
    return owner != NULL ? inference_select_result(inference, owner, node->as.control.condition, &value)
        : semantic_fail(inference->program, node->as.control.condition->span, "The condition's operation prefix has not been typed", NULL);
}
/** Process one event exactly once; syntax classification never changes after failure. */
static bool event(Inference *inference, const InferenceEvent *current) {
    InferenceValue value;
    switch (current->kind) {
        case INFER_DECLARATION:
            return inference_encounter(inference, current->node->as.declaration.name->name, NULL);
        case INFER_INITIALIZER:
            return inference_source(inference, current->source, true, &value);
        case INFER_ASSIGNMENT:
            return inference_assignment(inference, current->node);
        case INFER_CALL:
            return inference_operation(inference, current->node);
        case INFER_BINDING:
            return inference_binding(inference, current);
        case INFER_CONDITION:
            return condition(inference, current->node);
        case INFER_FOREACH:
            return inference_foreach(inference, current->node);
    }
    return semantic_fail(inference->program, current->site, "An inference event is outside its defined domain", NULL);
}
SemanticStatus semantic_infer(SemanticProgram *program) {
    assert(program->names_valid && program->native_valid && !program->inference_attempted);
    program->inference_attempted = true;
    if (!semantic_link_types(program)) { return program->status; }
    for (SemanticType *type = program->first; type != NULL; type = type->next) {
        Inference inference = {.program = program, .type = type};
        if (!inference_collect(&inference)) { return program->status; }
        inference_sort_events(&inference);
        for (const InferenceEvent *current = inference.first; current != NULL; current = current->next) {
            if (!event(&inference, current)) { return program->status; }
        }
        inference_order_fields(type);
    }
    program->inference_valid = true;
    return SEMANTIC_OK;
}
