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
