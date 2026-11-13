/**
 * @file
 * @brief Compose nominal linking, clone capability and typed initializer/statement resolution.
 *
 * [COMP-02](../../../spec/implementation/compiler.md#comp-02)
 * Resolve against complete explicit names before graph validation; no partially resolved program is published.
 * [TYPE-06](../../../spec/language/declarations.md#type-06) Inference completes before copy capability and transfer validation.
 */
#include <assert.h>
#include "semantic/internal.h"

SemanticStatus semantic_resolve(SemanticProgram *program) {
    assert(program->native_valid && !program->resolve_attempted);
    program->resolve_attempted = true;
    if (!program->inference_attempted && semantic_infer(program) != SEMANTIC_OK) {
        return program->status;
    }
    if (!program->inference_valid) { return program->status; }
    semantic_clone_capabilities(program);
    for (SemanticType *type = program->first; type != NULL; type = type->next) {
        if (!semantic_initializers(program, type) || !semantic_statements(program, type)) {
            return program->status;
        }
    }
    program->resolved = true;
    return SEMANTIC_OK;
}
