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
    bool typed_declarations; /* Target front-end only; not the numeric backend. */
} StatementParser;

bool parse_module_body(StatementParser* parser);
bool parse_typed_declaration_section(Lexer* lexer, AstModule* module, AstDeclarationKind kind);
bool parse_declaration_section(Lexer* lexer, AstModule* module, AstDeclarationKind kind);

#endif
