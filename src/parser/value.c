#include "parser/internal.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

bool parser_error(StatementParser* parser, uint32_t line_number, const char* format, ...) {
    error_set(parser->error, ERROR_INVALID, line_number, "");
    snprintf(parser->error->module, sizeof(parser->error->module), "%s", parser->module_path);
    va_list arguments;
    va_start(arguments, format);
    vsnprintf(parser->error->message, sizeof(parser->error->message), format, arguments);
    va_end(arguments);
    return false;
}

Symbol* require_value(StatementParser* parser, const char* name, uint32_t line_number) {
    Symbol* symbol = lookup_symbol(parser->symbols, parser->module_path, name);
    if (!symbol || symbol->kind == SYMBOL_MODULE_ALIAS) {
        parser_error(parser, line_number, "unknown numeric name '%s'", name);
        return NULL;
    }
    return symbol;
}

Symbol* get_or_create_receiver(StatementParser* parser, const char* name, uint32_t line_number) {
    Symbol* symbol = lookup_symbol(parser->symbols, parser->module_path, name);
    if (symbol && symbol->kind == SYMBOL_MODULE_ALIAS) {
        parser_error(parser, line_number, "alias '%s' cannot be used as a receiver", name);
        return NULL;
    }
    return symbol ? symbol : add_local_variable(parser->symbols, parser->module_path, name, parser->error);
}

bool emit_parsed_value(
    StatementParser* parser,
    Token value,
    uint32_t destination_symbol_id,
    const char* role
) {
    if (value.type == TOKEN_LITERAL) {
        emit_constant_assignment(parser->emitter, destination_symbol_id, atoi(value.lexeme));
        return true;
    }
    if (value.type == TOKEN_IDENTIFIER) {
        Symbol* symbol = require_value(parser, value.lexeme, value.line_number);
        if (!symbol) return false;
        emit_copy(parser->emitter, destination_symbol_id, symbol->symbol_id);
        return true;
    }
    return parser_error(parser, value.line_number, "invalid %s (token type %d)", role, (int)value.type);
}

bool parse_call_parameter(StatementParser* parser, uint32_t destination_symbol_id) {
    Token value = token_stream_advance(parser->stream);
    emit_source_location(parser->emitter, token_location(value));
    return emit_parsed_value(parser, value, destination_symbol_id, "parameter value");
}
