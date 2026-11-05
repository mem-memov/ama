#include "parser/internal.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

bool parser_error(CodeParser* P, uint32_t line, const char* fmt, ...) {
    error_set(P->error, ERROR_INVALID, line, "");
    snprintf(P->error->module, sizeof(P->error->module), "%s", P->module_path);
    va_list args;
    va_start(args, fmt);
    vsnprintf(P->error->message, sizeof(P->error->message), fmt, args);
    va_end(args);
    return false;
}

Symbol* require_value(CodeParser* P, const char* name, uint32_t line) {
    Symbol* symbol = lookup_symbol(P->symbols, P->module_path, name);
    if (!symbol || symbol->kind == SYM_ALIAS) {
        parser_error(P, line, "unknown numeric name '%s'", name);
        return NULL;
    }
    return symbol;
}

Symbol* get_or_create_receiver(CodeParser* P, const char* name, uint32_t line) {
    Symbol* symbol = lookup_symbol(P->symbols, P->module_path, name);
    if (symbol && symbol->kind == SYM_ALIAS) {
        parser_error(P, line, "alias '%s' cannot be used as a receiver", name);
        return NULL;
    }
    return symbol ? symbol : add_local_variable(P->symbols, P->module_path, name, P->error);
}

bool emit_parsed_value(CodeParser* P, Token value, uint32_t dst, const char* role) {
    if (value.type == TOK_LITERAL) {
        emit_assign_const(P->emitter, dst, atoi(value.lexeme));
        return true;
    }
    if (value.type == TOK_IDENT) {
        Symbol* symbol = require_value(P, value.lexeme, value.line);
        if (!symbol) return false;
        emit_copy(P->emitter, dst, symbol->id);
        return true;
    }
    return parser_error(P, value.line, "invalid %s (token type %d)", role, (int)value.type);
}

bool parse_call_param(CodeParser* P, uint32_t dst) {
    return emit_parsed_value(P, ts_next(P->stream), dst, "parameter value");
}
