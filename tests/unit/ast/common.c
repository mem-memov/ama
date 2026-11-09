#ifndef AMA_TESTS_AST_COMMON_C
#define AMA_TESTS_AST_COMMON_C
#include "../common.c"
#include "ast/ast.h"

static AstModule module;

static void setup_test(void) {
    test_count++;
    error_clear(&error);
    module = (AstModule){0};
}

static void destroy_test(void) {
    ast_destroy(&module);
}
#endif
