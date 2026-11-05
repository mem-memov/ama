#include "parser/internal.h"
#include <stdlib.h>

static void parse_body(CodeParser* P, uint32_t n) {
    while (ts_peek(P->stream)->type != TOK_EOF &&
           ts_peek(P->stream)->type != TOK_ERROR &&
           ts_peek(P->stream)->indent == n) {
        parse_statement(P, n);
    }

    Token* next = ts_peek(P->stream);
    if (next->type != TOK_EOF && next->type != TOK_ERROR &&
        next->indent > n) {
        fatal_at(P->module_path, next->line,
                    "unexpected indent %u (expected %u or less): "
                    "indent cannot jump by more than one level",
                    next->indent, n);
    }
}

void parse_branch(CodeParser* P, uint32_t n) {
    Token cond = ts_next(P->stream);
    if (cond.type != TOK_IDENT)
        fatal_at(P->module_path, cond.line, "branch condition must be an identifier");
    Symbol* cs = require_value(P, cond.lexeme, cond.line);

    Token q = ts_next(P->stream);
    if (q.type != TOK_QUESTION || q.indent != n || !q.is_bol
            || q.is_after_blank)
        fatal_at(P->module_path, q.line,
                    "expected '?' at indent %u immediately after condition",
                    n);

    emit_branch_start(cs->id);
    parse_body(P, n + 1);

    Token* peek = ts_peek(P->stream);
    if (peek->type == TOK_COLON && peek->indent == n && peek->is_bol) {
        ts_next(P->stream);
        emit_branch_else();
        parse_body(P, n + 1);
    }

    emit_branch_end();
}

void parse_loop(CodeParser* P, uint32_t n) {
    Token cond = ts_next(P->stream);
    if (cond.type != TOK_IDENT)
        fatal_at(P->module_path, cond.line, "loop condition must be an identifier");
    Symbol* cs = require_value(P, cond.lexeme, cond.line);

    Token lb = ts_next(P->stream);
    if (lb.type != TOK_LBRACE || lb.indent != n || !lb.is_bol
            || lb.is_after_blank)
        fatal_at(P->module_path, lb.line,
                    "expected '{' at indent %u immediately after condition",
                    n);

    emit_loop_start(cs->id);
    parse_body(P, n + 1);

    Token rb = ts_next(P->stream);
    if (rb.type != TOK_RBRACE || rb.indent != n)
        fatal_at(P->module_path, rb.line, "expected '}' at indent %u (got indent %u)",
                    n, rb.indent);

    emit_loop_end();
}
