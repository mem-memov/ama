/**
 * @file
 * @brief Resolve literal, local and qualified-constant sources into nominal value views.
 *
 * [TYPE-05](../../../spec/language/declarations.md#type-05)
 * Sources preserve original spans and declared storage form.
 * [REF-01](../../../spec/language/values-and-references.md#ref-01) gives self
 * sources an enclosing-instance meaning; their typing remains T032, with an explicit unsupported diagnostic until then.
 */
#include "semantic/internal.h"

/** Resolve an exact qualified shared member, its scalar/list form and initializer visibility; no namespace fallback or instance is created.
 */
static bool constant(SemanticProgram *program, SemanticSource *source,
    const SemanticSymbol *initializing) {
    const AstSource *syntax = source->syntax;
    SemanticType *owner = semantic_type(program, syntax->module_path);
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

SemanticSource *semantic_source(SemanticProgram *program, SemanticType *type,
    const AstSource *syntax, const SemanticSymbol *initializing) {
    SemanticSource *source = semantic_allocate(program, sizeof(*source));
    if (source == NULL) {
        return NULL;
    }
    source->syntax = syntax;
    source->site = syntax->span;
    source->kind = syntax->kind;
    if (syntax->kind == AST_LITERAL_SOURCE) {
        return source;
    }
    if (syntax->kind == AST_CONSTANT_SOURCE) {
        return constant(program, source, initializing) ? source : NULL;
    }
    /* T032 supplies lexical-self typing and later plans supply its checked
     * instance identity. A parsed self source intentionally has no named
     * access, so reject this unsupported stage before any access resolution. */
    if (syntax->kind == AST_SELF_REFERENCE_SOURCE) {
        (void)semantic_fail(program, syntax->span,
            "Self-reference sources are not supported by this compiler yet", initializing);
        program->error.code = DIAGNOSTIC_UNSUPPORTED_FEATURE;
        return NULL;
    }
    source->access = semantic_access(program, type, syntax->access, false, initializing);
    if (source->access == NULL) {
        return NULL;
    }
    SemanticSymbol *symbol = source->access->symbol;
    source->type = symbol->type;
    source->declaration = symbol;
    source->constant = symbol->role == SYMBOL_CONSTANT ? symbol : NULL;
    source->reference = source->access->reference;
    source->list = source->access->list;
    if (symbol->role == SYMBOL_CONSTANT && syntax->kind != AST_VALUE_SOURCE) {
        semantic_fail(program, syntax->span, "Constants cannot supply mutable references or continuations", symbol);
        return NULL;
    }
    if (syntax->kind == AST_ACQUIRE_SOURCE) {
        source->reference = true;
    }
    return source;
}
