/**
 * @file
 * @brief Compose nominal linking, clone capability and typed initializer/statement resolution.
 *
 * [COMP-02](../../../spec/implementation/compiler.md#comp-02)
 * Resolve against complete explicit names before graph validation; no partially resolved program is published.
 */
#include <assert.h>
#include "semantic/internal.h"

SemanticStatus semantic_resolve(SemanticProgram *program) {
    assert(program->native_valid && !program->resolve_attempted);
    program->resolve_attempted = true;
    if (!semantic_link_types(program)) {
        return program->status;
    }
    semantic_clone_capabilities(program);
    for (SemanticType *type = program->first; type != NULL; type = type->next) {
        if (!semantic_initializers(program, type) || !semantic_statements(program, type)) {
            return program->status;
        }
    }
    program->resolved = true;
    return SEMANTIC_OK;
}
