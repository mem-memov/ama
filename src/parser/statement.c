#include "parser/internal.h"
#include <string.h>

static bool require_line_end(StatementParser* parser, Token token, const char* role) {
    const Token* next = token_stream_peek(parser->stream);
    return next->type == TOKEN_END_OF_FILE || next->line_number != token.line_number ||
        parser_error(parser, next->line_number, "%s must end its line", role);
}

bool parse_statement(StatementParser* parser, uint32_t indentation, AstBlock* block) {
    Token name = token_stream_advance(parser->stream);
    if (name.type == TOKEN_ERROR) return false;
    if (name.type == TOKEN_CALL_MARKER)
        return parser_error(parser, name.line_number,
            "standalone '!' is not a statement; use '^' to exit a loop");
    if (name.indentation != indentation)
        return parser_error(parser, name.line_number, "expected indent %u, got %u", indentation, name.indentation);
    AstStatement* statement = ast_allocate(parser->module, sizeof(*statement), parser->error);
    if (!statement) return false;
    statement->location = token_location(name);
    ast_append_statement(block, statement);
    if (name.type == TOKEN_BREAK) {
        statement->kind = AST_BREAK;
        return require_line_end(parser, name, "loop exit '^'");
    }
    if (name.type == TOKEN_RESULT_ARROW) {
        statement->kind = AST_RETURN;
        return require_line_end(parser, name, "return '>'");
    }
    if (name.type != TOKEN_IDENTIFIER)
        return parser_error(parser, name.line_number, "unexpected token type %d at statement start", (int)name.type);
    Token* next = token_stream_peek(parser->stream);
    if (parser->typed_declarations && (next->type == TOKEN_AMPERSAND || next->type == TOKEN_LIST_OPEN ||
        (next->type == TOKEN_MODULE_PATH && next->is_at_line_start && next->indentation == indentation))) {
        statement->kind = AST_DECLARATION;
        return parse_typed_declaration(parser, name, indentation, AST_BODY_DECLARATION,
            &statement->as.declaration);
    }
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
    if (next->type == TOKEN_CALL_MARKER) {
        if (next->line_number != name.line_number ||
            next->column_number != name.column_number + strlen(name.lexeme))
            return parser_error(parser, next->line_number,
                "call marker '!' must immediately follow its target on the same line");
        Token marker = token_stream_advance(parser->stream);
        if (!require_line_end(parser, marker, "call target and '!'")) return false;
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
