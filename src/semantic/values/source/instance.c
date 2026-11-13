/**
 * @file
 * @brief Resolve named caller sources and explicit acquisition/continuation modes without implicit dereference.
 * [TYPE-05](../../../../spec/language/declarations.md#type-05) keeps nominal
 * identity and storage form exact; immutable shared values cannot supply mutable references.
 */
#include "internal.h"
bool semantic_instance_source(SemanticProgram *program, SemanticType *type,
    SemanticSource *source, const SemanticSymbol *initializing) {
    const AstSource *syntax = source->syntax;
    source->access = semantic_access(program, type, syntax->access, false, initializing);
    if (source->access == NULL) {
        return false;
    }
    SemanticSymbol *symbol = source->access->symbol;
    source->type = symbol->type;
    source->declaration = symbol;
    source->constant = symbol->role == SYMBOL_CONSTANT ? symbol : NULL;
    source->reference = source->access->reference;
    source->list = source->access->list;
    if (symbol->role == SYMBOL_CONSTANT && syntax->kind != AST_VALUE_SOURCE) {
        semantic_fail(program, syntax->span, "Constants cannot supply mutable references or continuations", symbol);
        return false;
    }
    if (syntax->kind == AST_ACQUIRE_SOURCE) {
        source->reference = true;
    }
    return true;
}
