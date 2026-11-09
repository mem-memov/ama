#include "parser/internal.h"

typedef enum {
    STATEMENT_ERROR, STATEMENT_END, STATEMENT_BREAK, STATEMENT_RETURN, STATEMENT_BRANCH, STATEMENT_LOOP,
    STATEMENT_CALL, STATEMENT_ASSIGNMENT
} StatementKind;

/* Conditions bind greedily to the next structural marker at the same indent. */
static StatementKind classify_statement(StatementParser* parser, uint32_t indentation) {
    Token* token = token_stream_peek(parser->stream);
    if (token->type == TOKEN_END_OF_FILE)
        return STATEMENT_END;
    if (token->type == TOKEN_ERROR) {
        parser_error(parser, token->line_number, "lexical error");
        return STATEMENT_ERROR;
    }
    if (token->indentation != indentation) {
        parser_error(parser, token->line_number,
            "expected indent %u, got %u", indentation, token->indentation);
        return STATEMENT_ERROR;
    }
    if (token->type == TOKEN_BREAK)
        return STATEMENT_BREAK;
    if (token->type == TOKEN_RESULT_ARROW)
        return STATEMENT_RETURN;
    if (token->type != TOKEN_IDENTIFIER) {
        parser_error(parser, token->line_number,
            "unexpected token type %d at statement start", (int)token->type);
        return STATEMENT_ERROR;
    }

    Token* next = token_stream_peek_next(parser->stream);
    if (next->indentation == indentation && next->is_at_line_start &&
        (next->type == TOKEN_QUESTION_MARK || next->type == TOKEN_LOOP_OPEN)) {
        if (next->has_preceding_blank_line) {
            parser_error(parser, next->line_number,
                "blank line between a condition and '%s' is not "
                "allowed; remove the blank line",
                next->type == TOKEN_QUESTION_MARK ? "?" : "{");
            return STATEMENT_ERROR;
        }
        return next->type == TOKEN_QUESTION_MARK ? STATEMENT_BRANCH : STATEMENT_LOOP;
    }
    Symbol* symbol = lookup_symbol(parser->symbols, parser->module_path, token->lexeme);
    return symbol && symbol->kind == SYMBOL_MODULE_ALIAS ? STATEMENT_CALL : STATEMENT_ASSIGNMENT;
}

bool parse_statement(StatementParser* parser, uint32_t indentation) {
    emit_source_location(parser->emitter, token_location(*token_stream_peek(parser->stream)));
    switch (classify_statement(parser, indentation)) {
    case STATEMENT_ERROR:
        return false;
    case STATEMENT_END:
        return true;
    case STATEMENT_BREAK:
        token_stream_advance(parser->stream);
        emit_break(parser->emitter);
        return true;
    case STATEMENT_RETURN:
        token_stream_advance(parser->stream);
        emit_return(parser->emitter);
        return true;
    case STATEMENT_BRANCH:
        return parse_branch(parser, indentation);
    case STATEMENT_LOOP:
        return parse_loop(parser, indentation);
    case STATEMENT_CALL: {
        Token name = token_stream_advance(parser->stream);
        Symbol* alias = lookup_symbol(parser->symbols, parser->module_path, name.lexeme);
        return parse_call(parser, indentation, alias, token_location(name));
    }
    case STATEMENT_ASSIGNMENT: {
        Token name = token_stream_advance(parser->stream);
        Symbol* destination_symbol = get_or_create_receiver(parser, name.lexeme, name.line_number);
        return destination_symbol && parse_assignment(parser, indentation, destination_symbol->symbol_id);
    }
    }
    return false;
}
