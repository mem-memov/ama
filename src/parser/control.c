#include "parser/internal.h"
#include <stdlib.h>

static bool parse_body(StatementParser* parser, uint32_t indentation) {
    while (token_stream_peek(parser->stream)->type != TOKEN_END_OF_FILE &&
        token_stream_peek(parser->stream)->type != TOKEN_ERROR &&
        token_stream_peek(parser->stream)->indentation == indentation) {
        if (!parse_statement(parser, indentation)) return false;
    }

    Token* next_token = token_stream_peek(parser->stream);
    if (next_token->type != TOKEN_END_OF_FILE && next_token->type != TOKEN_ERROR &&
        next_token->indentation > indentation) {
        return parser_error(parser, next_token->line_number,
            "unexpected indent %u (expected %u or less): "
            "indent cannot jump by more than one level",
            next_token->indentation, indentation);
    }
    return true;
}

bool parse_branch(StatementParser* parser, uint32_t indentation) {
    Token condition_token = token_stream_advance(parser->stream);
    if (condition_token.type != TOKEN_IDENTIFIER)
        return parser_error(parser, condition_token.line_number, "branch condition must be an identifier");
    Symbol* condition_symbol = require_value(parser, condition_token.lexeme, condition_token.line_number);
    if (!condition_symbol) return false;

    Token question_mark = token_stream_advance(parser->stream);
    if (question_mark.type != TOKEN_QUESTION_MARK || question_mark.indentation != indentation || !question_mark.is_at_line_start
        || question_mark.has_preceding_blank_line)
        return parser_error(parser, question_mark.line_number,
            "expected '?' at indent %u immediately after condition",
            indentation);

    emit_branch_start(parser->emitter, condition_symbol->symbol_id);
    if (!parse_body(parser, indentation + 1)) return false;

    Token* next_token = token_stream_peek(parser->stream);
    if (next_token->type == TOKEN_COLON && next_token->indentation == indentation && next_token->is_at_line_start) {
        Token alternative = token_stream_advance(parser->stream);
        emit_source_location(parser->emitter, token_location(alternative));
        emit_branch_else(parser->emitter);
        if (!parse_body(parser, indentation + 1)) return false;
    }

    emit_generated_location(parser->emitter);
    emit_branch_end(parser->emitter);
    return true;
}

bool parse_loop(StatementParser* parser, uint32_t indentation) {
    Token condition_token = token_stream_advance(parser->stream);
    if (condition_token.type != TOKEN_IDENTIFIER)
        return parser_error(parser, condition_token.line_number, "loop condition must be an identifier");
    Symbol* condition_symbol = require_value(parser, condition_token.lexeme, condition_token.line_number);
    if (!condition_symbol) return false;

    Token loop_open = token_stream_advance(parser->stream);
    if (loop_open.type != TOKEN_LOOP_OPEN || loop_open.indentation != indentation || !loop_open.is_at_line_start
        || loop_open.has_preceding_blank_line)
        return parser_error(parser, loop_open.line_number,
            "expected '{' at indent %u immediately after condition",
            indentation);

    emit_loop_start(parser->emitter, condition_symbol->symbol_id);
    if (!parse_body(parser, indentation + 1)) return false;

    Token loop_close = token_stream_advance(parser->stream);
    if (loop_close.type != TOKEN_LOOP_CLOSE || loop_close.indentation != indentation)
        return parser_error(parser, loop_close.line_number, "expected '}' at indent %u (got indent %u)",
            indentation, loop_close.indentation);

    emit_source_location(parser->emitter, token_location(loop_close));
    emit_loop_end(parser->emitter);
    return true;
}
