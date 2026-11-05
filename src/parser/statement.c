#include "parser/internal.h"

typedef enum {
    STMT_ERROR, STMT_END, STMT_BREAK, STMT_RETURN, STMT_BRANCH, STMT_LOOP,
    STMT_CALL, STMT_ASSIGN
} StatementKind;

/* Conditions bind greedily to the next structural marker at the same indent. */
static StatementKind classify_statement(CodeParser* P, uint32_t indent) {
    Token* token = ts_peek(P->stream);
    if (token->type == TOK_EOF)
        return STMT_END;
    if (token->type == TOK_ERROR) {
        parser_error(P, token->line, "lexical error");
        return STMT_ERROR;
    }
    if (token->indent != indent) {
        parser_error(P, token->line,
                 "expected indent %u, got %u", indent, token->indent);
        return STMT_ERROR;
    }
    if (token->type == TOK_BANG)
        return STMT_BREAK;
    if (token->type == TOK_GREATER)
        return STMT_RETURN;
    if (token->type != TOK_IDENT) {
        parser_error(P, token->line,
                 "unexpected token type %d at statement start", (int)token->type);
        return STMT_ERROR;
    }

    Token* next = ts_peek2(P->stream);
    if (next->indent == indent && next->is_bol &&
        (next->type == TOK_QUESTION || next->type == TOK_LBRACE)) {
        if (next->is_after_blank) {
            parser_error(P, next->line,
                     "blank line between a condition and '%s' is not "
                     "allowed; remove the blank line",
                     next->type == TOK_QUESTION ? "?" : "{");
            return STMT_ERROR;
        }
        return next->type == TOK_QUESTION ? STMT_BRANCH : STMT_LOOP;
    }
    Symbol* symbol = lookup_symbol(P->symbols, P->module_path, token->lexeme);
    return symbol && symbol->kind == SYM_ALIAS ? STMT_CALL : STMT_ASSIGN;
}

bool parse_statement(CodeParser* P, uint32_t indent) {
    switch (classify_statement(P, indent)) {
    case STMT_ERROR:
        return false;
    case STMT_END:
        return true;
    case STMT_BREAK:
        ts_next(P->stream);
        emit_break(P->emitter);
        return true;
    case STMT_RETURN:
        ts_next(P->stream);
        emit_return(P->emitter);
        return true;
    case STMT_BRANCH:
        return parse_branch(P, indent);
    case STMT_LOOP:
        return parse_loop(P, indent);
    case STMT_CALL: {
        Token name = ts_next(P->stream);
        Symbol* alias = lookup_symbol(P->symbols, P->module_path, name.lexeme);
        return parse_call(P, indent, alias, name.line);
    }
    case STMT_ASSIGN: {
        Token name = ts_next(P->stream);
        Symbol* dst = get_or_create_receiver(P, name.lexeme, name.line);
        return dst && parse_assign(P, indent, dst->id);
    }
    }
    return false;
}
