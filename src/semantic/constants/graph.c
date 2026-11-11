#include "internal.h"

bool semantic_constant_graph(SemanticProgram *program) {
    for (SemanticType *type = program->first; type != NULL; type = type->next) {
        SemanticSymbol *previous = NULL;
        for (SemanticSymbol *symbol = type->first; symbol != NULL; symbol = symbol->next) {
            if (symbol->role != SYMBOL_CONSTANT) {
                continue;
            }
            GraphVertex *vertex = semantic_constant_vertex(&program->constants, symbol);
            if (vertex == NULL || (previous != NULL && !dependency_edge(&program->constants, vertex,
                    semantic_constant_vertex(&program->constants, previous),
                    symbol->declaration->as.declaration.name->span))) {
                return false;
            }
            previous = symbol;
        }
    }
    for (SemanticType *type = program->first; type != NULL; type = type->next) {
        for (SemanticSymbol *symbol = type->first; symbol != NULL; symbol = symbol->next) {
            if (symbol->role == SYMBOL_CONSTANT && !semantic_constant_construction(program, symbol)) {
                return false;
            }
        }
    }
    return true;
}
