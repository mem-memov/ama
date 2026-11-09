#include "parser/internal.h"

static bool parse_argument(StatementParser* parser, AstStatement* statement) {
    Token token = token_stream_advance(parser->stream);
    AstArgument* argument = ast_allocate(parser->module, sizeof(*argument), parser->error);
    if (!argument) return false;
    argument->location = token_location(token);
    argument->callee_symbol_id = AST_UNRESOLVED_SYMBOL;
    if (token.type == TOKEN_IDENTIFIER) {
        argument->is_named = true;
        argument->label = ast_name_from_token(token);
        token = token_stream_advance(parser->stream);
        if (token.type != TOKEN_PARAMETER_ARROW && token.type != TOKEN_RESULT_ARROW &&
            token.type != TOKEN_RETAINED_RESULT_ARROW)
            return parser_error(parser, token.line_number, "expected '<', '>', or '>>' after '%s'", argument->label.text);
    } else if (token.type != TOKEN_PARAMETER_ARROW && token.type != TOKEN_RESULT_ARROW &&
            token.type != TOKEN_RETAINED_RESULT_ARROW) {
        return parser_error(parser, token.line_number,
            "unexpected token type %d in call arguments", (int)token.type);
    }
    argument->kind = token.type == TOKEN_PARAMETER_ARROW ? AST_PARAMETER_ARGUMENT :
        token.type == TOKEN_RETAINED_RESULT_ARROW ? AST_RETAINED_RESULT_ARGUMENT : AST_RESULT_ARGUMENT;
    Token value = token_stream_advance(parser->stream);
    if (argument->kind != AST_PARAMETER_ARGUMENT && value.type != TOKEN_IDENTIFIER)
        return parser_error(parser, value.line_number, "expected identifier after '%s'", token.lexeme);
    if (!parse_value(parser, value, &argument->value, "parameter value")) return false;
    if (statement->as.call.last_argument) statement->as.call.last_argument->next = argument;
    else statement->as.call.arguments = argument;
    statement->as.call.last_argument = argument;
    return true;
}

bool parse_call(StatementParser* parser, uint32_t indentation, AstStatement* statement) {
    while (token_stream_peek(parser->stream)->type != TOKEN_END_OF_FILE &&
        token_stream_peek(parser->stream)->type != TOKEN_ERROR &&
        token_stream_peek(parser->stream)->indentation == indentation + 1)
        if (!parse_argument(parser, statement)) return false;
    return true;
}
