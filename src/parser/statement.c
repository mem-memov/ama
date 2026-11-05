#include "parser/internal.h"

typedef enum {
    STMT_END, STMT_BREAK, STMT_RETURN, STMT_BRANCH, STMT_LOOP,
    STMT_CALL, STMT_ASSIGN
} StatementKind;

/* Conditions bind greedily to the next structural marker at the same indent. */
static StatementKind classify_statement(CodeParser* P, uint32_t indent) {
    Token* token = ts_peek(P->stream);
    if (token->type == TOK_EOF)
        return STMT_END;
    if (token->type == TOK_ERROR)
        fatal_at(P->module_path, token->line, "lexical error");
    if (token->indent != indent)
        fatal_at(P->module_path, token->line,
                 "expected indent %u, got %u", indent, token->indent);
    if (token->type == TOK_BANG)
        return STMT_BREAK;
    if (token->type == TOK_GREATER)
        return STMT_RETURN;
    if (token->type != TOK_IDENT)
        fatal_at(P->module_path, token->line,
                 "unexpected token type %d at statement start", (int)token->type);

    Token* next = ts_peek2(P->stream);
    if (next->indent == indent && next->is_bol &&
        (next->type == TOK_QUESTION || next->type == TOK_LBRACE)) {
        if (next->is_after_blank)
            fatal_at(P->module_path, next->line,
                     "blank line between a condition and '%s' is not "
                     "allowed; remove the blank line",
                     next->type == TOK_QUESTION ? "?" : "{");
        return next->type == TOK_QUESTION ? STMT_BRANCH : STMT_LOOP;
    }
    Symbol* symbol = lookup_symbol(P->module_path, token->lexeme);
    return symbol && symbol->kind == SYM_ALIAS ? STMT_CALL : STMT_ASSIGN;
}

void parse_statement(CodeParser* P, uint32_t indent) {
    switch (classify_statement(P, indent)) {
    case STMT_END:
        return;
    case STMT_BREAK:
        ts_next(P->stream);
        emit_break();
        return;
    case STMT_RETURN:
        ts_next(P->stream);
        emit_return();
        return;
    case STMT_BRANCH:
        parse_branch(P, indent);
        return;
    case STMT_LOOP:
        parse_loop(P, indent);
        return;
    case STMT_CALL: {
        Token name = ts_next(P->stream);
        Symbol* alias = lookup_symbol(P->module_path, name.lexeme);
        parse_call(P, indent, alias, name.line);
        return;
    }
    case STMT_ASSIGN: {
        Token name = ts_next(P->stream);
        Symbol* dst = get_or_create_receiver(P, name.lexeme, name.line);
        parse_assign(P, indent, dst->id);
        return;
    }
    }
}
