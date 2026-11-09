#ifndef AMA_PARSER_PARSER_H
#define AMA_PARSER_PARSER_H

#include "parser/stream.h"
#include "lexer/lexer.h"
#include "ast/ast.h"

typedef struct {
    TokenStream* stream;
    const char* module_path;
    AstModule* module;
    Error* error;
    unsigned int nesting;
} StatementParser;

bool parse_module_body(StatementParser* parser);
bool parse_declaration_section(Lexer* lexer, AstModule* module, AstDeclarationKind kind);

#endif
