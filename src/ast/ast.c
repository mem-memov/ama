#include "ast/ast.h"
#include <stddef.h>
#include <stdlib.h>

struct AstAllocation {
    struct AstAllocation* next;
    max_align_t alignment;
};

void* ast_allocate(AstModule* module, size_t size, Error* error) {
    if (size > SIZE_MAX - sizeof(AstAllocation)) {
        error_set(error, ERROR_LIMIT, 0, "AST allocation size exceeded");
        return NULL;
    }
    AstAllocation* allocation = calloc(1, sizeof(*allocation) + size);
    if (!allocation) {
        error_set(error, ERROR_MEMORY, 0, "out of memory (AST)");
        return NULL;
    }
    allocation->next = module->allocations;
    module->allocations = allocation;
    module->validated = false;
    module->declarations_validated = false;
    return allocation + 1;
}

void ast_destroy(AstModule* module) {
    AstAllocation* allocation = module->allocations;
    while (allocation) {
        AstAllocation* next = allocation->next;
        free(allocation);
        allocation = next;
    }
    *module = (AstModule){0};
}

void ast_append_statement(AstBlock* block, AstStatement* statement) {
    if (block->last) block->last->next = statement;
    else block->first = statement;
    block->last = statement;
}

void ast_append_declaration(AstModule* module, AstDeclaration* declaration) {
    if (module->last_declaration) module->last_declaration->next = declaration;
    else module->declarations = declaration;
    module->last_declaration = declaration;
}
