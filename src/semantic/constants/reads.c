#include "internal.h"

static bool require_constant(SemanticProgram *program, SemanticSymbol *owner,
    SemanticSymbol *target, SourceSpan site) {
    return dependency_edge(&program->constants, semantic_constant_vertex(&program->constants, owner),
        semantic_constant_vertex(&program->constants, target), site);
}

bool semantic_constant_reads(SemanticProgram *program, SemanticSymbol *owner,
    const SemanticSource *source) {
    if (source->constant != NULL && !require_constant(program, owner, source->constant, source->site)) {
        return false;
    }
    for (SemanticAccess *access = source->access; access != NULL; access = access->index) {
        if (access->symbol->role == SYMBOL_CONSTANT
            && !require_constant(program, owner, access->symbol, access->syntax->span)) {
            return false;
        }
    }
    if (source->kind == AST_ACQUIRE_SOURCE) {
        return semantic_fail(program, source->site,
            "Shared value construction cannot create a mutable-object reference", owner);
    }
    return true;
}
