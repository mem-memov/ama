#include "parser/internal.h"

bool parse_statement(StatementParser* parser, uint32_t indentation, AstBlock* block) {
    Token name = token_stream_advance(parser->stream);
    if (name.type == TOKEN_ERROR) return false;
    if (name.indentation != indentation)
        return parser_error(parser, name.line_number, "expected indent %u, got %u", indentation, name.indentation);
    AstStatement* statement = ast_allocate(parser->module, sizeof(*statement), parser->error);
    if (!statement) return false;
    statement->location = token_location(name);
    ast_append_statement(block, statement);
    if (name.type == TOKEN_BREAK) {
        statement->kind = AST_BREAK;
        return true;
    }
    if (name.type == TOKEN_RESULT_ARROW) {
        statement->kind = AST_RETURN;
        return true;
    }
    if (name.type != TOKEN_IDENTIFIER)
        return parser_error(parser, name.line_number, "unexpected token type %d at statement start", (int)name.type);
    Token* next = token_stream_peek(parser->stream);
    if (next->indentation == indentation && next->is_at_line_start &&
        (next->type == TOKEN_QUESTION_MARK || next->type == TOKEN_LOOP_OPEN)) {
        if (next->has_preceding_blank_line)
            return parser_error(parser, next->line_number,
                "blank line between a condition and '%s' is not allowed; remove the blank line",
                next->type == TOKEN_QUESTION_MARK ? "?" : "{");
        if (!parse_value(parser, name, &statement->as.control.condition, "condition")) return false;
        statement->kind = next->type == TOKEN_QUESTION_MARK ? AST_BRANCH : AST_LOOP;
        return statement->kind == AST_BRANCH ? parse_branch(parser, indentation, statement) :
            parse_loop(parser, indentation, statement);
    }
    /* The current language needs declared alias names to distinguish a zero-arg
        call from assignment. This is syntax context, not symbol resolution.
    */
    if (ast_has_alias(parser->module, name.lexeme)) {
        statement->kind = AST_CALL;
        statement->as.call.target = ast_name_from_token(name);
        return parse_call(parser, indentation, statement);
    }
    statement->kind = AST_ASSIGNMENT;
    statement->as.assignment.receiver = ast_name_from_token(name);
    return parse_assignment(parser, indentation, statement);
}

bool parse_module_body(StatementParser* parser) {
    if (!parse_body(parser, 0, &parser->module->body)) return false;
    Token* end = token_stream_peek(parser->stream);
    return end->type == TOKEN_END_OF_FILE ||
        parser_error(parser, end->line_number, "unexpected token at module scope");
}
