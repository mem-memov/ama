#ifndef AMA_PARSER_INTERNAL_H
#define AMA_PARSER_INTERNAL_H

#include "parser/parser.h"
#include "symbols/table.h"
#include "common/diagnostic.h"
#include "codegen/emitter.h"

bool parser_error(StatementParser* parser, uint32_t line_number, const char* format, ...);
Symbol* require_value(StatementParser* parser, const char* name, uint32_t line_number);
Symbol* get_or_create_receiver(StatementParser* parser, const char* name, uint32_t line_number);
bool emit_parsed_value(
    StatementParser* parser,
    Token value,
    uint32_t destination_symbol_id,
    const char* role
);
bool parse_assignment(StatementParser* parser, uint32_t indentation, uint32_t destination_symbol_id);
bool parse_call_parameter(StatementParser* parser, uint32_t destination_symbol_id);
bool parse_call(StatementParser* parser, uint32_t indentation, Symbol* alias, SourceLocation location);
bool parse_branch(StatementParser* parser, uint32_t indentation);
bool parse_loop(StatementParser* parser, uint32_t indentation);

#endif
