#ifndef AMA_PARSER_INTERNAL_H
#define AMA_PARSER_INTERNAL_H
#include "parser/parser.h"

bool parser_error(StatementParser* parser, uint32_t line_number, const char* format, ...);
AstName ast_name_from_token(Token token);
bool parse_value(StatementParser* parser, Token token, AstValue* value, const char* role);
bool parse_statement(StatementParser* parser, uint32_t indentation, AstBlock* block);
bool parse_body(StatementParser* parser, uint32_t indentation, AstBlock* block);
bool parse_assignment(StatementParser* parser, uint32_t indentation, AstStatement* statement);
bool parse_call(StatementParser* parser, uint32_t indentation, AstStatement* statement);
bool parse_branch(StatementParser* parser, uint32_t indentation, AstStatement* statement);
bool parse_loop(StatementParser* parser, uint32_t indentation, AstStatement* statement);
#endif
