#include "semantic.h"

SemanticStatus semantic_analyze(SemanticProgram *program, SemanticLibraries libraries) {
    if (semantic_names(program) != SEMANTIC_OK || semantic_native(program) != SEMANTIC_OK
        || semantic_resolve(program, libraries) != SEMANTIC_OK
        || semantic_relations(program) != SEMANTIC_OK || semantic_constants(program) != SEMANTIC_OK) {
        return program->status;
    }
    return SEMANTIC_OK;
}
