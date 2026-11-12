/**
 * @file
 * @brief Own counted structural AST fixtures and source lifetime through parser failures.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../docs/implementation/library-tests-and-build.md)
 */
#ifndef AMA_AST_TEST_COMMON
#define AMA_AST_TEST_COMMON
#include "../source/common.c"
#include "ast/ast.h"

static AstModule module;

/**
 * Initialize counted source and AST owners in dependency order before parsing.
 */
static void setup_tree(void) {
    setup_test();
    ast_module_init(&module, sources.allocator);
}

/**
 * Register exact fixture bytes in stable owned storage and return the actual structural module
 * parse status.
 */
static ParseStatus parse_text(const char *text) {
    const SourceText *source;
    SourceStatus added = source_manager_add(&sources, "module", (const unsigned char *)text,
        strlen(text), &source);
    assert(added == SOURCE_OK);
    return parser_module_parse(&module, source);
}

/**
 * Destroy AST before source storage and assert no counted fixture allocation remains.
 */
static void destroy_tree(void) {
    ast_module_destroy(&module);
    destroy_test();
}
#endif
