/**
 * @file
 * @brief Resolve exact immutable qualified sources independently from local/self identity.
 * [CONST-02](../../../../spec/language/declarations.md#const-02) requires shared
 * namespace membership and preserves original scalar/list and initialization checks.
 * [CONST-05](../../../../spec/language/declarations.md#const-05) reuses the exact
 * selected declaration and actual member origin from closed source evidence.
 */
#include "internal.h"
#include "semantic/specializations/namespace.h"

/** Resolve an exact qualified shared member, its scalar/list form and initializer visibility; no namespace fallback or instance is created.
 * Component contract: [CONST-02](../../../../spec/language/declarations.md#const-02). */
bool semantic_constant_source(SemanticProgram *program, SemanticType *caller, SemanticSource *source,
    const SemanticSymbol *initializing) {
    const AstSource *syntax = source->syntax;
    SourceSpan member = {0};
    SemanticSymbol *symbol = semantic_qualified_constant(program, caller, syntax, &member);
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
    source->constant_member = member;
    source->declaration = symbol;
    source->type = symbol->type;
    source->list = symbol->list;
    return true;
}
