/**
 * @file
 * @brief Resolve literal, local and qualified-constant sources into nominal value views.
 *
 * [TYPE-05](../../../spec/language/declarations.md#type-05)
 * Sources preserve original spans and declared storage form.
 * [REF-01](../../../spec/language/values-and-references.md#ref-01) gives self
 * sources the exact lexical module identity, independently of destination or dynamic caller.
 */
#include "source/internal.h"

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
        return semantic_constant_source(program, source, initializing) ? source : NULL;
    }
    if (syntax->kind == AST_SELF_REFERENCE_SOURCE) {
        return semantic_self_source(program, type, source, initializing) ? source : NULL;
    }
    return semantic_instance_source(program, type, source, initializing) ? source : NULL;
}
