#ifndef AMA_PARSER_INTERNAL_H
#define AMA_PARSER_INTERNAL_H

#include "parser/parser.h"
#include "symbols/table.h"
#include "common/diagnostic.h"
#include "codegen/emitter.h"

bool parser_error(CodeParser* P, uint32_t line, const char* fmt, ...);
Symbol* require_value(CodeParser* P, const char* name, uint32_t line);
Symbol* get_or_create_receiver(CodeParser* P, const char* name, uint32_t line);
bool emit_parsed_value(CodeParser* P, Token value, uint32_t dst, const char* role);
bool parse_assign(CodeParser* P, uint32_t indent, uint32_t dst);
bool parse_call_param(CodeParser* P, uint32_t dst);
bool parse_call(CodeParser* P, uint32_t indent, Symbol* alias, uint32_t line);
bool parse_branch(CodeParser* P, uint32_t indent);
bool parse_loop(CodeParser* P, uint32_t indent);

#endif
