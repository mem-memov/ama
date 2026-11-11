#ifndef AMA_AST_TEST_COMMON
#define AMA_AST_TEST_COMMON
#include "../source/common.c"
#include "ast/ast.h"

static AstModule module;

static void setup_tree(void) {
    setup_test();
    ast_module_init(&module, sources.allocator);
}

static ParseStatus parse_text(const char *text) {
    const SourceText *source;
    SourceStatus added = source_manager_add(&sources, "module", (const unsigned char *)text,
        strlen(text), &source);
    assert(added == SOURCE_OK);
    return parser_module_parse(&module, source);
}

static void destroy_tree(void) {
    ast_module_destroy(&module);
    destroy_test();
}
#endif
