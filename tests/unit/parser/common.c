#ifndef AMA_TESTS_PARSER_COMMON_C
#define AMA_TESTS_PARSER_COMMON_C
#include "../common.c"
#include "parser/parser.h"
#include "semantic/semantic.h"
#include "codegen/ast.h"
static SymbolTable* symbols;
static Graph* graph;
static Emitter emitter;
static AstModule syntax;
static FILE* output;
static void setup_test(void) {
    test_count++;
    error_clear(&error);
    syntax = (AstModule){0};
    symbols = malloc(sizeof(*symbols));
    graph = malloc(sizeof(*graph));
    output = tmpfile();
    assert(symbols != NULL && graph != NULL && output != NULL);
    symbols_init(symbols);
    graph_init(graph);
    emitter_init(&emitter, output, symbols);
    emitter.module_path = "main";
}
static void destroy_test(void) {
    ast_destroy(&syntax);
    close_file(output);
    free(graph);
    free(symbols);
}
static bool parse_syntax_at(const char* text, const char* source_path) {
    FILE* source = text_file(text);
    Lexer lexer;
    bool ready = init_lexer(&lexer, source, source_path, 0, (long)strlen(text), 1, &error);
    assert(ready);
    TokenStream stream = {0};
    bool success = read_token_stream(&lexer, &stream);
    close_file(source);
    StatementParser parser = {.stream = &stream, .module_path = "main",
        .module = &syntax, .error = &error};
    if (success) success = parse_module_body(&parser);
    destroy_token_stream(&stream); /* No AST spelling may borrow token storage. */
    return success;
}
static bool parse_text_at(const char* text, const char* source_path) {
    return parse_syntax_at(text, source_path) &&
        validate_module_body(&syntax, symbols, graph, "main", &error) &&
        emit_ast_body(&emitter, &syntax, &error);
}
static bool parse_text(const char* text) {
    return parse_text_at(text, NULL);
}
#endif
