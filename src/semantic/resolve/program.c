#include <assert.h>
#include "semantic/internal.h"

static bool configure(SemanticProgram *program, SemanticLibraries libraries) {
    const char *paths[] = {libraries.action, libraries.index, libraries.condition, libraries.outcome};
    const char *owned[4] = {0};
    for (size_t index = 0; index < 4; index++) {
        program->library_types[index] = semantic_find_type(program, paths[index]);
        if (paths[index] != NULL) {
            owned[index] = source_arena_copy(&program->arena, paths[index]);
            if (owned[index] == NULL) {
                return semantic_memory_failure(program);
            }
        }
    }
    program->libraries = (SemanticLibraries){owned[0], owned[1], owned[2], owned[3]};
    return true;
}

SemanticStatus semantic_resolve(SemanticProgram *program, SemanticLibraries libraries) {
    assert(program->native_valid && !program->resolve_attempted);
    program->resolve_attempted = true;
    if (!configure(program, libraries) || !semantic_link_types(program)) {
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
