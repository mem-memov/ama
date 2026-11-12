/**
 * @file
 * @brief Initialize and release complete or failed structural syntax owners.
 *
 * [ARCH-03](../../spec/implementation/compiler.md#arch-03)
 * Nodes/layout belong to explicit arenas; their text spans borrow the source manager.
 */
#include "ast.h"

void ast_module_init(AstModule *module, SourceAllocator allocator) {
    *module = (AstModule){0};
    layout_init(&module->layout, allocator);
    source_arena_init(&module->arena, allocator);
}

void ast_module_destroy(AstModule *module) {
    source_arena_destroy(&module->arena);
    layout_destroy(&module->layout);
    module->valid = false;
}
