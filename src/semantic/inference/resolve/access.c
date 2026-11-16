/**
 * @file
 * @brief Type and encounter caller accesses without guessing storage suffixes or future inferred fields.
 * [TYPE-06](../../../../spec/language/declarations.md#type-06) visits each
 * named index in source order; [NAME-06](../../../../spec/language/declarations.md#name-06)
 * resolves lexical locals before module fields and forbids construction-time local access.
 */
#include "../internal.h"
/** Resolve one base name after its actual occurrence is reached; a current inference cannot depend on itself. Component contract: [TYPE-06](../../../../spec/language/declarations.md#type-06), [NAME-06](../../../../spec/language/declarations.md#name-06). */
SemanticSymbol *inference_name(Inference *inference, SourceSpan name, SourceSpan site, bool construction) {
    SemanticSymbol *symbol = semantic_local_at(inference->type, name, site);
    if (symbol != NULL) {
        if (construction) {
            semantic_fail(inference->program, name, "Instance construction cannot read a runtime foreach-local binding", symbol);
            return NULL;
        }
        return symbol;
    }
    if (!inference_encounter(inference, name, NULL)) { return NULL; }
    if (inference->inferring.source != NULL && semantic_same_text(inference->inferring, name)) {
        semantic_fail(inference->program, name, "A field cannot depend on itself while its type is being inferred", NULL);
        return NULL;
    }
    symbol = semantic_symbol(inference->type, name);
    if (symbol == NULL) {
        semantic_fail(inference->program, name, "The variable has no explicit or earlier inferred declaration", NULL);
    }
    return symbol;
}
bool inference_access(Inference *inference, const AstAccess *syntax,
    bool construction, InferenceValue *value) {
    bool outer = true;
    for (const AstAccess *node = syntax; node != NULL; node = node->index) {
        SemanticSymbol *symbol = inference_name(inference, node->name, node->span, construction);
        if (symbol == NULL) { return false; }
        if (node->reference != symbol->reference || (node->whole_list || node->index != NULL) != symbol->list) {
            return semantic_fail(inference->program, node->span, "Access storage form does not match its declaration", symbol);
        }
        if (outer) {
            *value = (InferenceValue){symbol->type, symbol, symbol->reference, node->whole_list};
        } else if (symbol->reference || node->whole_list || !symbol->type->abi.bindings[ABI_INDEX].present) {
            return semantic_fail(inference->program, node->span, "The index must be an owned scalar with the [] capability", symbol);
        }
        outer = false;
    }
    return !outer || semantic_fail(inference->program, inference->type->module->requirement, "A typed access requires named syntax", NULL);
}
