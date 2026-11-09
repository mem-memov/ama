#include "parser/internal.h"
#include <stdarg.h>
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

AstName ast_name_from_token(Token token) {
    AstName name = {.location = token_location(token), .symbol_id = AST_UNRESOLVED_SYMBOL};
    memcpy(name.text, token.lexeme, sizeof(name.text));
    return name;
}

bool parse_value(StatementParser* parser, Token token, AstValue* value, const char* role) {
    if (token.type != TOKEN_IDENTIFIER && token.type != TOKEN_LITERAL)
        return parser_error(parser, token.line_number, "invalid %s (token type %d)", role, (int)token.type);
    *value = (AstValue){.kind = token.type == TOKEN_LITERAL ? AST_LITERAL_VALUE : AST_NAME_VALUE,
        .location = token_location(token), .symbol_id = AST_UNRESOLVED_SYMBOL};
    memcpy(value->text, token.lexeme, sizeof(value->text));
    return true;
}
