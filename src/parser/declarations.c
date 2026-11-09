#include "parser/internal.h"
#include <string.h>

bool parse_declaration_section(Lexer* lexer, AstModule* module, AstDeclarationKind kind) {
    for (;;) {
        Token name = get_next_token(lexer);
        if (name.type == TOKEN_ERROR) return false;
        if (name.type == TOKEN_END_OF_FILE) return true;
        if (name.type != TOKEN_IDENTIFIER || !name.is_at_line_start || name.indentation != 0) {
            destroy_token(&name);
            return error_set(lexer->error, ERROR_INVALID, name.line_number, "expected a variable name at indent 0");
        }
        Token value = get_next_token(lexer);
        if (value.type == TOKEN_ERROR) return false;
        if (value.type == TOKEN_END_OF_FILE)
            return error_set(lexer->error, ERROR_INVALID, name.line_number,
                "variable '%s' has no value", name.lexeme);
        if (!value.is_at_line_start || value.indentation != 0) {
            destroy_token(&value);
            return error_set(lexer->error, ERROR_INVALID, value.line_number,
                "value for '%s' must be at indent 0", name.lexeme);
        }
        if (value.type != TOKEN_LITERAL && value.type != TOKEN_IDENTIFIER && value.type != TOKEN_MODULE_PATH) {
            destroy_token(&value);
            return error_set(lexer->error, ERROR_INVALID, value.line_number,
                "invalid declaration default; use a literal or an alias path");
        }
        AstDeclaration* declaration = ast_allocate(module, sizeof(*declaration), lexer->error);
        if (!declaration) return false;
        declaration->kind = kind;
        declaration->name = ast_name_from_token(name);
        declaration->value = (AstValue){.kind = value.type == TOKEN_MODULE_PATH ? AST_PATH_VALUE :
            value.type == TOKEN_LITERAL ? AST_LITERAL_VALUE : AST_NAME_VALUE,
            .location = token_location(value), .symbol_id = AST_UNRESOLVED_SYMBOL};
        memcpy(declaration->value.text, value.lexeme, sizeof(declaration->value.text));
        ast_append_declaration(module, declaration);
    }
}
