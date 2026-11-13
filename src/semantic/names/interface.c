/**
 * @file
 * @brief Share exact interface field lookup across bindings and subsequent result inference.
 * [CALL-08](../../../spec/language/calls-and-results.md#call-08) separates
 * direction ordinals from names; [TYPE-06](../../../spec/language/declarations.md#type-06)
 * consumes the same declared result metadata for inference in its later stage.
 */
#include "semantic/internal.h"
SemanticSymbol *semantic_interface_name(const SemanticType *type, SymbolRole role, SourceSpan name) {
    if (role < SYMBOL_CONSTANT || role > SYMBOL_RESULT) { return NULL; }
    SemanticSymbol *symbol = semantic_symbol(type, name);
    return symbol != NULL && symbol->role == role ? symbol : NULL;
}
SemanticSymbol *semantic_interface_position(const SemanticType *type, SymbolRole role, size_t position) {
    if (role < SYMBOL_CONSTANT || role > SYMBOL_RESULT) { return NULL; }
    for (SemanticSymbol *symbol = type->first; symbol != NULL; symbol = symbol->next) {
        if (symbol->role == role && symbol->role_ordinal == position) { return symbol; }
    }
    return NULL;
}
