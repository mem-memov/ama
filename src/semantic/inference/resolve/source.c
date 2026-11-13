/**
 * @file
 * @brief Derive exact assignment source type and transfer form while visiting its caller occurrences.
 * [TYPE-06](../../../../spec/language/declarations.md#type-06) specifies each
 * source form; runtime values, literal contents and callee bodies never supply an inferred type.
 */
#include "../internal.h"
bool inference_source(Inference *inference, const AstSource *source,
    bool construction, InferenceValue *value) {
    if (source->kind == AST_LITERAL_SOURCE) { *value = (InferenceValue){0}; return true; }
    if (source->kind == AST_SELF_REFERENCE_SOURCE) {
        *value = (InferenceValue){.type = inference->type, .reference = true};
        return true;
    }
    if (source->kind == AST_CONSTANT_SOURCE) {
        SemanticType *owner = semantic_type(inference->program, source->module_path);
        SemanticSymbol *symbol = owner == NULL ? NULL : semantic_interface_name(owner, SYMBOL_CONSTANT, source->member);
        if (symbol == NULL || symbol->list != source->constant_list) {
            return semantic_fail(inference->program, source->span, "The namespace has no matching shared constant", symbol);
        }
        *value = (InferenceValue){symbol->type, symbol, false, symbol->list};
        return true;
    }
    if (!inference_access(inference, source->access, construction, value)) { return false; }
    if (value->symbol->role == SYMBOL_CONSTANT && source->kind != AST_VALUE_SOURCE) {
        return semantic_fail(inference->program, source->span, "Constants cannot supply mutable references or continuations", value->symbol);
    }
    if (source->kind == AST_ACQUIRE_SOURCE) { value->reference = true; }
    if (source->kind == AST_CONTINUATION_SOURCE) { value->reference = false; value->list = false; }
    return true;
}
