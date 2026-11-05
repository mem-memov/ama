#include "parser/parser.h"

bool parse_declaration_section(Lexer* lexer, SymbolTable* symbols,
                                const char* module, SymbolKind kind) {
    for (;;) {
        Token name = get_next_token(lexer);
        if (name.type == TOK_ERROR) return false;
        if (name.type == TOK_EOF) return true;
        if (name.type != TOK_IDENT || !name.is_bol || name.indent != 0)
            return error_set(lexer->error, ERROR_INVALID, name.line,
                             "expected a variable name at indent 0");
        Token value = get_next_token(lexer);
        if (value.type == TOK_ERROR) return false;
        if (value.type == TOK_EOF)
            return error_set(lexer->error, ERROR_INVALID, name.line,
                             "variable '%s' has no value", name.lexeme);
        if (!value.is_bol || value.indent != 0)
            return error_set(lexer->error, ERROR_INVALID, value.line,
                             "value for '%s' must be at indent 0", name.lexeme);
        SymbolKind actual = value.type == TOK_PATH ? SYM_ALIAS : kind;
        if (!add_declarative_symbol(symbols, module, name.lexeme, actual, value, lexer->error))
            return false;
    }
}
