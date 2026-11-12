/**
 * @file
 * @brief Compose semantic stages in their explicit prerequisite order.
 *
 * [COMP-02](../../spec/implementation/compiler.md#comp-02)
 * Names, native contracts, value resolution, relations and shared plans must each succeed before consumption.
 */
#include "semantic.h"

SemanticStatus semantic_analyze_authoring(SemanticProgram *program,
    const SemanticNativeAuthoring *authoring) {
    if (semantic_names(program) != SEMANTIC_OK || semantic_native_authoring(program, authoring) != SEMANTIC_OK
        || semantic_resolve(program) != SEMANTIC_OK
        || semantic_relations(program) != SEMANTIC_OK || semantic_constants(program) != SEMANTIC_OK) {
        return program->status;
    }
    return SEMANTIC_OK;
}

SemanticStatus semantic_analyze(SemanticProgram *program) {
    return semantic_analyze_authoring(program, NULL);
}
