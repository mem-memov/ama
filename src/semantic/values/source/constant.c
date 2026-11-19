/**
 * @file
 * @brief Resolve exact immutable qualified sources independently from local/self identity.
 * [CONST-02](../../../../spec/language/declarations.md#const-02) requires shared
 * namespace membership and preserves original scalar/list and initialization checks.
 */
#include "internal.h"
#include "semantic/specializations/namespace.h"

/** Resolve an exact qualified shared member, its scalar/list form and initializer visibility; no namespace fallback or instance is created.
 * Component contract: [CONST-02](../../../../spec/language/declarations.md#const-02). */
bool semantic_constant_source(SemanticProgram *program, SemanticType *caller, SemanticSource *source,
    const SemanticSymbol *initializing) {
    const AstSource *syntax = source->syntax;
    SemanticType *owner = semantic_qualified_namespace(program, caller, syntax);
    SemanticSymbol *symbol = owner == NULL ? NULL : semantic_symbol(owner, syntax->member);
    if (symbol == NULL || symbol->role != SYMBOL_CONSTANT) {
        return semantic_fail(program, syntax->span, "The namespace has no such shared constant", symbol);
    }
    if (syntax->constant_list != symbol->list) {
        return semantic_fail(program, syntax->span, "Constant storage form does not match its declaration", symbol);
    }
    if (!semantic_visible(program, symbol, initializing, syntax->span)) {
        return false;
    }
    source->constant = symbol;
    source->declaration = symbol;
    source->type = symbol->type;
    source->list = symbol->list;
    return true;
}
