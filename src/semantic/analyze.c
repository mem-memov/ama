#include "semantic.h"

SemanticStatus semantic_analyze_authoring(SemanticProgram *program, SemanticLibraries libraries,
    const SemanticNativeAuthoring *authoring) {
    if (semantic_names(program) != SEMANTIC_OK || semantic_native_authoring(program, authoring) != SEMANTIC_OK
        || semantic_resolve(program, libraries) != SEMANTIC_OK
        || semantic_relations(program) != SEMANTIC_OK || semantic_constants(program) != SEMANTIC_OK) {
        return program->status;
    }
    return SEMANTIC_OK;
}

SemanticStatus semantic_analyze(SemanticProgram *program, SemanticLibraries libraries) {
    return semantic_analyze_authoring(program, libraries, NULL);
}
