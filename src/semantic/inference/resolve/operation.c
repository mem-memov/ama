/**
 * @file
 * @brief Type finite root/selected paths through declared results and fixed typed list interfaces.
 * [CALL-09](../../../../spec/language/calls-and-results.md#call-09) excludes
 * selector base names from caller lookup; [TYPE-06](../../../../spec/language/declarations.md#type-06)
 * still visits their index operands in the initiating caller.
 */
#include "../internal.h"
InferenceOperation *inference_find_operation(const Inference *inference, const AstNode *node) {
    for (InferenceOperation *operation = inference->operations; operation != NULL; operation = operation->next) {
        if (operation->syntax == node) { return operation; }
    }
    return NULL;
}
/** Match methods only after a typed list-service parent is known; these names are declared interface entries. */
static bool method(Inference *inference, const AstNode *node,
    const InferenceOperation *parent, InferenceOperation *operation) {
    const AstAccess *target = node->as.call.target;
    if (!semantic_list_method(target->name, &operation->method)) {
        return semantic_fail(inference->program, target->name, "The list service has no such method", NULL);
    }
    if (target->reference || target->whole_list || target->index != NULL || node->as.call.mode != AST_ENTRY_CALL) {
        return semantic_fail(inference->program, target->span, "A list method requires its plain declared name and entry mode", NULL);
    }
    operation->kind = INFER_LIST_METHOD;
    operation->value = parent->value;
    operation->value.list = false;
    return true;
}
bool inference_operation(Inference *inference, const AstNode *node) {
    InferenceOperation *operation = semantic_allocate(inference->program, sizeof(*operation));
    if (operation == NULL) { return false; }
    operation->syntax = node;
    if (node->as.call.mode == AST_CONSTANT_READ) {
        operation->kind = INFER_NAMESPACE;
        operation->value.type = semantic_type(inference->program, node->as.call.namespace_path);
        if (operation->value.type == NULL) { return semantic_fail(inference->program, node->span, "The constant namespace is unavailable", NULL); }
    } else if (node->as.call.enclosing_operation != NULL) {
        InferenceOperation *parent = inference_find_operation(inference, node->as.call.enclosing_operation);
        if (parent == NULL) { return semantic_fail(inference->program, node->span, "The enclosing operation has not been typed", NULL); }
        if (parent->kind == INFER_LIST_SERVICE) {
            if (!method(inference, node, parent, operation)) { return false; }
        } else {
            if (!inference_select_result(inference, parent, node->as.call.target, &operation->value)) { return false; }
            operation->kind = operation->value.list ? INFER_LIST_SERVICE : INFER_INSTANCE;
        }
    } else {
        if (!inference_access(inference, node->as.call.target, false, &operation->value)) { return false; }
        if (operation->value.symbol->role == SYMBOL_CONSTANT) {
            return semantic_fail(inference->program, node->span, "Shared constants cannot be executable or retained-result targets", operation->value.symbol);
        }
        operation->kind = operation->value.list ? INFER_LIST_SERVICE : INFER_INSTANCE;
    }
    if (operation->kind == INFER_LIST_SERVICE && (node->as.call.mode != AST_RETAINED_READ
            || node->as.call.input_count != 0 || node->as.call.output_count != 0 || !node->as.call.has_selected)) {
        return semantic_fail(inference->program, node->span, "A whole list service parent accepts only selected entry methods", operation->value.symbol);
    }
    operation->next = inference->operations;
    inference->operations = operation;
    return true;
}
