#ifndef AMA_PARSER_INTERNAL_H
#define AMA_PARSER_INTERNAL_H

#include "parser/parser.h"
#include "symbols/table.h"
#include "common/diagnostic.h"
#include "codegen/emitter.h"

Symbol* require_value(CodeParser* P, const char* name, uint32_t line);
Symbol* get_or_create_receiver(CodeParser* P, const char* name, uint32_t line);
void emit_parsed_value(CodeParser* P, Token value, uint32_t dst, const char* role);
void parse_assign(CodeParser* P, uint32_t indent, uint32_t dst);
void parse_call_param(CodeParser* P, uint32_t dst);
void parse_call(CodeParser* P, uint32_t indent, Symbol* alias, uint32_t line);
void parse_branch(CodeParser* P, uint32_t indent);
void parse_loop(CodeParser* P, uint32_t indent);

#endif
