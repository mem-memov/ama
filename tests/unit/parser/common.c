#ifndef AMA_TESTS_PARSER_COMMON_C
#define AMA_TESTS_PARSER_COMMON_C
#include "../common.c"
#include "parser/parser.h"
static SymbolTable* symbols;
static Graph* graph;
static Emitter emitter;
static FILE* output;
static void setup_test(void) {
    count++;
    error_clear(&error);
    symbols = malloc(sizeof(*symbols));
    graph = malloc(sizeof(*graph));
    output = tmpfile();
    assert(symbols != NULL && graph != NULL && output != NULL);
    symbols_init(symbols);
    graph_init(graph);
    emitter_init(&emitter, output, symbols);
}
static void destroy_test(void) {
    close_file(output);
    free(graph);
    free(symbols);
}
static bool parse_text(const char* text) {
    FILE* source = text_file(text);
    Lexer lexer;
    bool ready = init_lexer(&lexer, source, 0, (long)strlen(text), 1, &error);
    assert(ready);
    Token tokens[256];
    TokenStream stream = {tokens, 0, 0};
    for (;;) {
        assert(stream.size < 256);
        Token token = get_next_token(&lexer);
        tokens[stream.size++] = token;
        if (token.type == TOK_ERROR || token.type == TOK_EOF) break;
    }
    close_file(source);
    if (error.code != ERROR_NONE) return false;
    CodeParser parser = {&stream, "main", symbols, graph, &emitter, &error};
    while (ts_peek(&stream)->type != TOK_EOF)
        if (!parse_statement(&parser, 0)) return false;
    return true;
}
#endif
