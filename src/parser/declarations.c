#include "parser/internal.h"

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

/* Collect only write targets. Unknown read-only names must still be errors.
    Assignment sources are skipped so they cannot become accidental declarations.
    Full statement validation remains the responsibility of the normal parser.
*/
bool collect_body_variables(StatementParser* parser) {
    const TokenStream* stream = parser->stream;
    for (int index = 0; index + 1 < stream->token_count; index++) {
        const Token* token = &stream->tokens[index];
        const Token* next = &stream->tokens[index + 1];
        if (token->type == TOKEN_RESULT_ARROW && next->type == TOKEN_IDENTIFIER &&
            !next->is_at_line_start) {
            if (!get_or_create_receiver(parser, next->lexeme, next->line_number)) return false;
            index++;
        } else if (token->type == TOKEN_IDENTIFIER && token->is_at_line_start &&
            next->indentation == token->indentation &&
            (next->type == TOKEN_LITERAL || next->type == TOKEN_IDENTIFIER)) {
            Symbol* symbol = lookup_symbol(parser->symbols, parser->module_path, token->lexeme);
            if (symbol && symbol->kind == SYMBOL_MODULE_ALIAS) continue;
            if (!get_or_create_receiver(parser, token->lexeme, token->line_number)) return false;
            index++;
        }
    }
    return true;
}
