/**
 * @file
 * @brief Add concrete shared read edges and reject mutable reference construction.
 *
 * [CONST-01](../../../spec/language/declarations.md#const-01)
 * Immutable shared values cannot acquire mutable instance references, including
 * lexical self in freshly constructed descendants. Access/index reads contribute
 * each actual prerequisite once rather than repeating a named shared root.
 */
#include "internal.h"

/** Add the exact owner-to-required-value edge with original read site; allocation failure remains graph/semantic-owned.
 * Component contract: [CONST-01](../../../spec/language/declarations.md#const-01). */
static bool require_constant(SemanticProgram *program, SemanticSymbol *owner,
    SemanticSymbol *target, SourceSpan site) {
    return dependency_edge(&program->constants, semantic_constant_vertex(&program->constants, owner),
        semantic_constant_vertex(&program->constants, target), site);
}

bool semantic_constant_reads(SemanticProgram *program, SemanticSymbol *owner,
    const SemanticSource *source) {
    if (source->constant != NULL && source->access == NULL && !require_constant(program, owner, source->constant, source->site)) {
        return false;
    }
    for (SemanticAccess *access = source->access; access != NULL; access = access->index) {
        if (access->symbol->role == SYMBOL_CONSTANT
            && !require_constant(program, owner, access->symbol, access->syntax->span)) {
            return false;
        }
    }
    if (source->kind == AST_ACQUIRE_SOURCE || source->kind == AST_SELF_REFERENCE_SOURCE) {
        return semantic_fail(program, source->site,
            "Shared value construction cannot create a mutable-object reference", owner);
    }
    return true;
}
