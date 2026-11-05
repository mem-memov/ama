#include "parser/parser.h"

bool parse_declaration_section(Lexer* lexer, SymbolTable* symbols,
    const char* module, SymbolKind kind) {
    for (;;) {
        Token name = get_next_token(lexer);
        if (name.type == TOKEN_ERROR) return false;
        if (name.type == TOKEN_END_OF_FILE) return true;
        if (name.type != TOKEN_IDENTIFIER || !name.is_at_line_start || name.indentation != 0)
            return error_set(lexer->error, ERROR_INVALID, name.line_number,
                "expected a variable name at indent 0");
        Token value = get_next_token(lexer);
        if (value.type == TOKEN_ERROR) return false;
        if (value.type == TOKEN_END_OF_FILE)
            return error_set(lexer->error, ERROR_INVALID, name.line_number,
                "variable '%s' has no value", name.lexeme);
        if (!value.is_at_line_start || value.indentation != 0)
            return error_set(lexer->error, ERROR_INVALID, value.line_number,
                "value for '%s' must be at indent 0", name.lexeme);
        SymbolKind actual = value.type == TOKEN_MODULE_PATH ? SYMBOL_MODULE_ALIAS : kind;
        if (!add_declarative_symbol(symbols, module, name.lexeme, actual, value, lexer->error))
            return false;
    }
}
