#include "parser/internal.h"
#include <stdlib.h>

Symbol* require_value(CodeParser* P, const char* name, uint32_t line) {
    Symbol* s = lookup_symbol(P->module_path, name);
    if (!s)
        fatal_at(P->module_path, line, "unknown name '%s'", name);
    if (s->kind == SYM_ALIAS)
        fatal_at(P->module_path, line, "alias '%s' cannot be used as a value", name);
    return s;
}

Symbol* get_or_create_receiver(CodeParser* P, const char* name,
                                       uint32_t line) {
    Symbol* s = lookup_symbol(P->module_path, name);
    if (s && s->kind == SYM_ALIAS)
        fatal_at(P->module_path, line, "alias '%s' cannot be used as a receiver", name);
    if (!s)
        s = add_local_variable(P->module_path, name);
    return s;
}

void emit_parsed_value(CodeParser* P, Token value, uint32_t dst, const char* role) {
    switch (value.type) {
    case TOK_LITERAL:
        emit_assign_const(dst, atoi(value.lexeme));
        return;
    case TOK_IDENT: {
        Symbol* symbol = require_value(P, value.lexeme, value.line);
        emit_copy(dst, symbol->id);
        return;
    }
    default:
        fatal_at(P->module_path, value.line, "invalid %s (token type %d)",
                 role, (int)value.type);
    }
}

void parse_call_param(CodeParser* P, uint32_t dst) {
    emit_parsed_value(P, ts_next(P->stream), dst, "parameter value");
}
