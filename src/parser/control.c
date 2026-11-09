#include "parser/internal.h"

bool parse_body(StatementParser* parser, uint32_t indentation, AstBlock* block) {
    if (parser->nesting >= MAX_AST_NESTING)
        return parser_error(parser, token_stream_peek(parser->stream)->line_number,
            "statement nesting exceeds %d levels", MAX_AST_NESTING);
    parser->nesting++;
    bool success = true;
    while (token_stream_peek(parser->stream)->type != TOKEN_END_OF_FILE &&
        token_stream_peek(parser->stream)->type != TOKEN_ERROR &&
        token_stream_peek(parser->stream)->indentation == indentation) {
        if (!parse_statement(parser, indentation, block)) { success = false; break; }
    }
    Token* next = token_stream_peek(parser->stream);
    if (success && next->type == TOKEN_ERROR) success = false;
    if (success && next->type != TOKEN_END_OF_FILE && next->indentation > indentation)
        success = parser_error(parser, next->line_number,
            "unexpected indent %u (expected %u or less): indent cannot jump by more than one level",
            next->indentation, indentation);
    parser->nesting--;
    return success;
}

bool parse_branch(StatementParser* parser, uint32_t indentation, AstStatement* statement) {
    token_stream_advance(parser->stream); /* '?' was classified by the caller. */
    if (!parse_body(parser, indentation + 1, &statement->as.control.body)) return false;
    Token* next = token_stream_peek(parser->stream);
    if (next->type == TOKEN_COLON && next->indentation == indentation && next->is_at_line_start) {
        statement->as.control.has_alternative = true;
        statement->as.control.alternative_location = token_location(token_stream_advance(parser->stream));
        if (!parse_body(parser, indentation + 1, &statement->as.control.alternative)) return false;
    }
    return true;
}

bool parse_loop(StatementParser* parser, uint32_t indentation, AstStatement* statement) {
    token_stream_advance(parser->stream); /* '{' was classified by the caller. */
    if (!parse_body(parser, indentation + 1, &statement->as.control.body)) return false;
    Token close = token_stream_advance(parser->stream);
    if (close.type != TOKEN_LOOP_CLOSE || close.indentation != indentation)
        return parser_error(parser, close.line_number, "expected '}' at indent %u (got indent %u)",
            indentation, close.indentation);
    statement->as.control.end_location = token_location(close);
    return true;
}
