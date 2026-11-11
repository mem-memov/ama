#include <assert.h>
#include <string.h>
#include "internal.h"

void semantic_program_init(SemanticProgram *program, const Project *project,
    SourceAllocator allocator) {
    assert(project->valid);
    *program = (SemanticProgram){.project = project};
    source_arena_init(&program->arena, allocator);
}

void semantic_program_destroy(SemanticProgram *program) {
    for (SemanticType *type = program->first; type != NULL; type = type->next) {
        abi_table_destroy(&type->abi);
    }
    source_arena_destroy(&program->arena);
    *program = (SemanticProgram){0};
}

void *semantic_allocate(SemanticProgram *program, size_t size) {
    void *result = source_arena_allocate(&program->arena, size);
    if (result == NULL) {
        semantic_memory_failure(program);
    } else {
        memset(result, 0, size);
    }
    return result;
}

bool semantic_same_text(SourceSpan left, SourceSpan right) {
    return left.end - left.start == right.end - right.start
        && memcmp(source_bytes(left.source) + left.start,
            source_bytes(right.source) + right.start, left.end - left.start) == 0;
}
