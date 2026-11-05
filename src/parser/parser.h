#ifndef AMA_PARSER_PARSER_H
#define AMA_PARSER_PARSER_H

#include "parser/stream.h"
#include "lexer/lexer.h"
#include "symbols/symbol.h"

typedef struct {
    TokenStream* stream;
    const char* module_path;
} CodeParser;

void parse_statement(CodeParser* P, uint32_t indent);
void parse_declaration_section(Lexer* L, const char* module_path, SymbolKind kind);

#endif
