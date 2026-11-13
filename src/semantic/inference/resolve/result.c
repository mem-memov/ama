/**
 * @file
 * @brief Select exact declared result metadata for receiver inference without inspecting callee bodies.
 * [TYPE-06](../../../../spec/language/declarations.md#type-06) shares ordinary
 * result labels/positions; [LIST-04](../../../../spec/language/lists.md#list-04)
 * fixes intrinsic element-reference and nominal integer result shapes.
 */
#include "../internal.h"
/** Match a fixed intrinsic result whose interface has no source declaration node. */
static bool intrinsic(Inference *inference, const InferenceOperation *operation,
    const AstAccess *label, size_t position, SourceSpan site, InferenceValue *value) {
    bool element = operation->method == INFER_APPEND || operation->method == INFER_INSERT;
    bool length = operation->method == INFER_LENGTH;
    if ((!element && !length) || (label == NULL ? position != 0
            : !layout_text_is(label->name, element ? "element" : "value"))) {
        return semantic_fail(inference->program, site, "No intrinsic result matches this binding", NULL);
    }
    SemanticType *type = element ? operation->value.type : semantic_find_type(inference->program, "..integer");
    if (type == NULL || !type->executable) {
        return semantic_fail(inference->program, site, "The typed list interface requires the ..integer module", NULL);
    }
    if (label != NULL && (label->reference != element || label->whole_list)) {
        return semantic_fail(inference->program, label->span, "Binding label storage form differs from its field", NULL);
    }
    *value = (InferenceValue){.type = type, .reference = element};
    return true;
}
bool inference_result(Inference *inference, const InferenceOperation *operation,
    const AstAccess *label, size_t position, SourceSpan site, InferenceValue *value) {
    if (operation->kind == INFER_LIST_METHOD) { return intrinsic(inference, operation, label, position, site, value); }
    if (operation->kind == INFER_LIST_SERVICE) {
        return semantic_fail(inference->program, site, "A list service parent has no ordinary result bindings", NULL);
    }
    SymbolRole role = operation->kind == INFER_NAMESPACE ? SYMBOL_CONSTANT : SYMBOL_RESULT;
    SemanticSymbol *symbol = label == NULL ? semantic_interface_position(operation->value.type, role, position)
        : semantic_interface_name(operation->value.type, role, label->name);
    if (symbol == NULL) {
        SemanticSymbol *related = label == NULL ? NULL : semantic_symbol(operation->value.type, label->name);
        return semantic_fail(inference->program, site, "No interface field matches this binding", related);
    }
    if (label != NULL && (label->reference != symbol->reference || label->whole_list != symbol->list)) {
        return semantic_fail(inference->program, label->span, "Binding label storage form differs from its field", symbol);
    }
    *value = (InferenceValue){symbol->type, symbol, symbol->reference, symbol->list};
    return true;
}
