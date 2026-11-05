#ifndef AMA_PARSER_PARSER_H
#define AMA_PARSER_PARSER_H

#include "parser/stream.h"
#include "lexer/lexer.h"
#include "symbols/table.h"
#include "graph/graph.h"
#include "codegen/emitter.h"

typedef struct {
    TokenStream* stream;
    const char* module_path;
    SymbolTable* symbols;
    const Graph* graph;
    Emitter* emitter;
    Error* error;
} CodeParser;

bool parse_statement(CodeParser* P, uint32_t indent);
bool parse_declaration_section(Lexer* L, SymbolTable* symbols, const char* module_path, SymbolKind kind);

#endif
