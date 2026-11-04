#include "ama.h"

#include <stdarg.h>

/* ============================================================================
 * Parser — overview
 * ============================================================================
 *
 * The parser receives a flat array of tokens (the code section of one module,
 * pre-lexed by the driver) and emits C code for the module body via the
 * emit_* functions.
 *
 * Grammar (indent is semantic):
 *
 *   statement   := assignment | deref_assign | call | branch | loop
 *                | break | return
 *
 *   assignment  := IDENT NEWLINE value
 *   deref_assign:= '@' IDENT NEWLINE value
 *   value       := LITERAL | IDENT | '@' IDENT | '&' IDENT
 *
 *   branch      := IDENT NEWLINE '?' NEWLINE body [ ':' NEWLINE body ]
 *   loop        := IDENT NEWLINE '{' NEWLINE body '}' NEWLINE
 *
 *   call        := IDENT NEWLINE { param_or_result }
 *   param_or_result
 *               := '<' value                 (positional parameter)
 *                | IDENT '<' value           (named parameter)
 *                | '>' IDENT                 (positional result)
 *                | IDENT '>' IDENT           (named result)
 *
 *   break       := '!' NEWLINE
 *   return      := '>' NEWLINE
 *   body        := statement*                    (all at indent n+1)
 *
 * Binding rules:
 *   - A condition (identifier) and its '?' or '{' must be at the SAME
 *     indent, on adjacent lines. No blank line between them, comments are
 *     allowed.
 *   - Assignment receiver and value must be at the SAME indent.
 *   - Call name is at indent n; parameters and results are at indent n+1.
 *   - Parameters must come before results.
 *   - A call uses either all positional arguments or all named ones; mixing
 *     is a hard error.
 *
 * Call emission order:
 *   1. emit_call_init(callee)       — reset all callee variables to defaults
 *   2. emit_call_param_* (each arg) — write arguments into callee slots
 *   3. emit_call_exec(callee)       — call the function
 *   4. emit_call_result (each res)  — copy result slots back to the caller
 *
 * Step 3 must run between writing parameters and reading results. Otherwise
 * results are read before the function has produced them (a silent bug that
 * returns stale/zero values). The parser emits the call lazily: on the first
 * result token, or after the argument loop if there are no results.
 *
 * Errors are hard: every failure prints a diagnostic and exits. The
 * compiler does not attempt to recover.
 *
 * Known limitations (documented, not enforced):
 *   - No arity check. Missing parameters default to 0 on each call.
 *   - Duplicate named arguments (left < 5, left < 7) are silently allowed;
 *     the last write wins.
 *   - Loop body does not need to update the condition; infinite loops are
 *     the programmer's responsibility.
 *   - Bare '>' return produces `return;` in a void function, which is
 *     fine, but it means the entry point always returns 0 to the OS.
 */

/* ============================================================================
 * Token stream
 * ============================================================================
 *
 * TokenStream is a flat array plus a cursor. All peeks are bounded: peeking
 * past the end returns the last token (always TOK_EOF, guaranteed by the
 * driver). This makes ts_next / ts_peek safe to call anywhere without
 * explicit length checks.
 */

Token* ts_peek(TokenStream* t) {
    if (t->pos >= t->size)
        return &t->data[t->size - 1];
    return &t->data[t->pos];
}

Token* ts_peek2(TokenStream* t) {
    if (t->pos + 1 >= t->size)
        return &t->data[t->size - 1];
    return &t->data[t->pos + 1];
}

Token ts_next(TokenStream* t) {
    if (t->pos >= t->size)
        return t->data[t->size - 1];
    return t->data[t->pos++];
}

/* ============================================================================
 * Error helper
 * ============================================================================ */

static void parse_error(CodeParser* P, uint32_t line, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "error: [%s] (line %u) ", P->module_path, line);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);
    exit(1);
}

/* ============================================================================
 * Symbol resolution
 * ============================================================================
 *
 * require_value: name must exist and must not be an alias. Used everywhere a
 * value is expected: assignment source, parameter value, condition, etc.
 * Aliases are compile-time indirections; treating one as a value would read
 * from storage[0] (nil).
 *
 * get_or_create_receiver: name may or may not exist. If it exists and is
 * an alias, that is an error. If it does not exist, it is created as a
 * local variable. Used for result receivers, which can implicitly declare
 * new locals — consistent with assignment.
 */

static Symbol* require_value(CodeParser* P, const char* name, uint32_t line) {
    Symbol* s = lookup_symbol(P->module_path, name);
    if (!s)
        parse_error(P, line, "unknown name '%s'", name);
    if (s->kind == SYM_ALIAS)
        parse_error(P, line, "alias '%s' cannot be used as a value", name);
    return s;
}

static Symbol* get_or_create_receiver(CodeParser* P, const char* name,
                                       uint32_t line) {
    Symbol* s = lookup_symbol(P->module_path, name);
    if (s && s->kind == SYM_ALIAS)
        parse_error(P, line, "alias '%s' cannot be used as a receiver", name);
    if (!s)
        s = add_local_variable(P->module_path, name);
    return s;
}

/* ============================================================================
 * Value parsing (shared by assignment and indirect assignment)
 * ============================================================================
 *
 * A value occupies one line at indent n. Four forms:
 *   LITERAL       — integer constant
 *   IDENT         — direct read
 *   '@' IDENT     — indirect read (pointer at IDENT's slot)
 *   '&' IDENT     — address of IDENT (its storage offset as a number)
 *
 * parse_value_into writes the value to storage[dst_off].
 * parse_value_indirect writes it to storage[storage[ptr_off]].
 *
 * The two differ only in which emit_* is called; the value grammar is the
 * same. Keeping them separate avoids passing a "mode" flag through every
 * branch, which is harder to read.
 */

static void parse_value_into(CodeParser* P, uint32_t n, uint32_t dst_off) {
    Token v = ts_next(P->stream);
    if (v.indent != n)
        parse_error(P, v.line,
                    "assignment source must be at indent %u, got %u",
                    n, v.indent);

    switch (v.type) {
    case TOK_LITERAL:
        emit_assign_const(dst_off, atoi(v.lexeme));
        return;
    case TOK_IDENT: {
        Symbol* s = require_value(P, v.lexeme, v.line);
        emit_assign_var(dst_off, s->offset);
        return;
    }
    case TOK_AT: {
        Token inner = ts_next(P->stream);
        if (inner.type != TOK_IDENT)
            parse_error(P, inner.line, "expected identifier after '@'");
        Symbol* s = require_value(P, inner.lexeme, inner.line);
        emit_assign_load_ind(dst_off, s->offset);
        return;
    }
    case TOK_AMP: {
        Token inner = ts_next(P->stream);
        if (inner.type != TOK_IDENT)
            parse_error(P, inner.line, "expected identifier after '&'");
        Symbol* s = require_value(P, inner.lexeme, inner.line);
        emit_assign_addr(dst_off, s->offset);
        return;
    }
    default:
        parse_error(P, v.line, "invalid assignment source (token type %d)",
                    (int)v.type);
    }
}

static void parse_value_indirect(CodeParser* P, uint32_t n, uint32_t ptr_off) {
    Token v = ts_next(P->stream);
    if (v.indent != n)
        parse_error(P, v.line,
                    "indirect assignment source must be at indent %u, got %u",
                    n, v.indent);

    switch (v.type) {
    case TOK_LITERAL:
        emit_store_indirect_const(ptr_off, atoi(v.lexeme));
        return;
    case TOK_IDENT: {
        Symbol* s = require_value(P, v.lexeme, v.line);
        emit_store_indirect_var(ptr_off, s->offset);
        return;
    }
    case TOK_AT: {
        Token inner = ts_next(P->stream);
        if (inner.type != TOK_IDENT)
            parse_error(P, inner.line, "expected identifier after '@'");
        Symbol* s = require_value(P, inner.lexeme, inner.line);
        emit_store_indirect_load_ind(ptr_off, s->offset);
        return;
    }
    case TOK_AMP: {
        Token inner = ts_next(P->stream);
        if (inner.type != TOK_IDENT)
            parse_error(P, inner.line, "expected identifier after '&'");
        Symbol* s = require_value(P, inner.lexeme, inner.line);
        emit_store_indirect_addr(ptr_off, s->offset);
        return;
    }
    default:
        parse_error(P, v.line,
                    "invalid indirect source (token type %d)", (int)v.type);
    }
}

/* ============================================================================
 * Statement parsers (forward declarations)
 * ============================================================================ */

static void parse_assign(CodeParser* P, uint32_t n, uint32_t dst_off);
static void parse_deref_assign(CodeParser* P, uint32_t n);
static void parse_branch(CodeParser* P, uint32_t n);
static void parse_loop(CodeParser* P, uint32_t n);
static void parse_call(CodeParser* P, uint32_t n, Symbol* alias,
                       uint32_t line);

/* ============================================================================
 * Simple statements
 * ============================================================================ */

static void parse_assign(CodeParser* P, uint32_t n, uint32_t dst_off) {
    parse_value_into(P, n, dst_off);
}

static void parse_deref_assign(CodeParser* P, uint32_t n) {
    /* Current token: '@' at indent n (already checked by dispatcher). */
    ts_next(P->stream);
    Token dst = ts_next(P->stream);
    if (dst.type != TOK_IDENT)
        parse_error(P, dst.line, "expected identifier after '@'");
    Symbol* ds = require_value(P, dst.lexeme, dst.line);
    parse_value_indirect(P, n, ds->offset);
}

/* ============================================================================
 * Body parsing
 * ============================================================================
 *
 * A body is a sequence of statements all at the same indent. It ends when
 * the next token has a different indent (shallower — the parent's ':' or
 * '}' — or deeper, which is a syntax error caught below).
 *
 * If the next token is at indent > n after the loop, the indentation jumped
 * by more than one level. That is a hard error: the language requires each
 * nested body to be exactly one level deeper than its parent. Reporting it
 * here (where we have the parent's indent in hand) produces a much clearer
 * message than letting the parent's ':' or '}' check fail later with an
 * unrelated error.
 */

static void parse_body(CodeParser* P, uint32_t n) {
    while (ts_peek(P->stream)->type != TOK_EOF &&
           ts_peek(P->stream)->type != TOK_ERROR &&
           ts_peek(P->stream)->indent == n) {
        parse_statement(P, n);
    }

    Token* next = ts_peek(P->stream);
    if (next->type != TOK_EOF && next->type != TOK_ERROR &&
        next->indent > n) {
        parse_error(P, next->line,
                    "unexpected indent %u (expected %u or less): "
                    "indent cannot jump by more than one level",
                    next->indent, n);
    }
}

/* ============================================================================
 * Branch
 * ============================================================================ */

static void parse_branch(CodeParser* P, uint32_t n) {
    Token cond = ts_next(P->stream);
    if (cond.type != TOK_IDENT)
        parse_error(P, cond.line, "branch condition must be an identifier");
    Symbol* cs = require_value(P, cond.lexeme, cond.line);

    Token q = ts_next(P->stream);
    if (q.type != TOK_QUESTION || q.indent != n || !q.is_bol
            || q.is_after_blank)
        parse_error(P, q.line,
                    "expected '?' at indent %u immediately after condition",
                    n);

    emit_branch_start(cs->offset);
    parse_body(P, n + 1);

    Token* peek = ts_peek(P->stream);
    if (peek->type == TOK_COLON && peek->indent == n && peek->is_bol) {
        ts_next(P->stream);
        emit_branch_else();
        parse_body(P, n + 1);
    }

    emit_branch_end();
}

/* ============================================================================
 * Loop
 * ============================================================================ */

static void parse_loop(CodeParser* P, uint32_t n) {
    Token cond = ts_next(P->stream);
    if (cond.type != TOK_IDENT)
        parse_error(P, cond.line, "loop condition must be an identifier");
    Symbol* cs = require_value(P, cond.lexeme, cond.line);

    Token lb = ts_next(P->stream);
    if (lb.type != TOK_LBRACE || lb.indent != n || !lb.is_bol
            || lb.is_after_blank)
        parse_error(P, lb.line,
                    "expected '{' at indent %u immediately after condition",
                    n);

    emit_loop_start(cs->offset);
    parse_body(P, n + 1);

    Token rb = ts_next(P->stream);
    if (rb.type != TOK_RBRACE || rb.indent != n)
        parse_error(P, rb.line, "expected '}' at indent %u (got indent %u)",
                    n, rb.indent);

    emit_loop_end();
}

/* ============================================================================
 * Call
 * ============================================================================
 *
 * Call structure:
 *   alias_name            (indent n)
 *   <value | ident<value | >dst | ident>dst    (indent n+1, 0+ lines)
 *
 * The parser:
 *   1. Resolves the alias to the callee module path.
 *   2. Verifies the callee exists (via the dependency graph, not the symbol
 *      table — see the note below).
 *   3. Emits initialization of all callee variables with their defaults.
 *   4. Parses parameters, writing each into the callee's slot.
 *   5. Emits the call itself — exactly once, before the first result.
 *   6. Parses results, copying each from the callee's slot to the caller's.
 *
 * Existence check: we use graph_has_node(), not module_exists().
 * module_exists() returns 0 for a module whose params/returns/locals are all
 * empty — such a module has no symbols in G_Table but still has a source
 * file and a generated C function. graph_has_node() is populated by the
 * driver from every scanned file, so it recognizes these modules correctly.
 *
 * Emission order matters:
 *   - parameters must be written before the call (else the callee reads
 *     stale/default values),
 *   - the call must run before results are read (else the caller copies
 *     stale/default result slots).
 * The "exec_emitted" flag ensures emit_call_exec runs exactly once, either
 * lazily on the first result token or once at the end if there are no
 * results at all.
 */

typedef enum { CALL_INIT, CALL_POSITIONAL, CALL_NAMED } CallMode;

/* Parse one parameter value (shared by positional and named forms). The
 * value is written to the callee's parameter slot at param_off. */
static void parse_call_param(CodeParser* P, uint32_t param_off) {
    Token v = ts_next(P->stream);
    switch (v.type) {
    case TOK_LITERAL:
        emit_call_param_const(param_off, atoi(v.lexeme));
        return;
    case TOK_IDENT: {
        Symbol* s = require_value(P, v.lexeme, v.line);
        emit_call_param_val(param_off, s->offset);
        return;
    }
    case TOK_AMP: {
        Token inner = ts_next(P->stream);
        if (inner.type != TOK_IDENT)
            parse_error(P, inner.line, "expected identifier after '&'");
        Symbol* s = require_value(P, inner.lexeme, inner.line);
        emit_call_param_addr(param_off, s->offset);
        return;
    }
    default:
        parse_error(P, v.line, "invalid parameter value (token type %d)",
                    (int)v.type);
    }
}

static void parse_call(CodeParser* P, uint32_t n, Symbol* alias,
                       uint32_t call_line) {
    char callee[MAX_PATH_LEN];
    normalize_path(alias->path_link, callee);

    if (!graph_has_node(callee))
        parse_error(P, call_line,
                    "module '%s' not found (called from '%s')",
                    callee, P->module_path);

    /* Phase 1: initialize all callee variables with their defaults. */
    emit_call_init(callee);

    CallMode mode = CALL_INIT;
    int seen_results = 0;
    int exec_emitted = 0;
    int pos_param_idx = 0;
    int pos_result_idx = 0;

    while (ts_peek(P->stream)->type != TOK_EOF &&
           ts_peek(P->stream)->type != TOK_ERROR &&
           ts_peek(P->stream)->indent == n + 1) {
        Token t = ts_next(P->stream);

        /* ------ positional form: leading '<' or '>' -------------------- */
        if (t.type == TOK_LESS || t.type == TOK_GREATER) {
            if (mode == CALL_NAMED)
                parse_error(P, t.line,
                            "cannot mix positional and named arguments");

            mode = CALL_POSITIONAL;

            if (t.type == TOK_LESS) {
                if (seen_results)
                    parse_error(P, t.line, "parameter after result");

                Symbol* cp = get_nth_symbol(callee, pos_param_idx++,
                                             SYM_PARAM);
                if (!cp)
                    parse_error(P, t.line,
                                "too many positional parameters for '%s'",
                                callee);
                parse_call_param(P, cp->offset);
            } else {
                seen_results = 1;

                /* Run the callee before reading any result. */
                if (!exec_emitted) {
                    emit_call_exec(callee);
                    exec_emitted = 1;
                }

                Symbol* cr = get_nth_symbol(callee, pos_result_idx++,
                                             SYM_RETURN);
                if (!cr)
                    parse_error(P, t.line,
                                "too many positional results for '%s'",
                                callee);

                Token dst = ts_next(P->stream);
                if (dst.type != TOK_IDENT)
                    parse_error(P, dst.line,
                                "expected identifier after '>'");
                Symbol* ds = get_or_create_receiver(P, dst.lexeme, dst.line);
                emit_call_result(ds->offset, cr->offset);
            }
            continue;
        }

        /* ------ named form: ident '<' value  OR  ident '>' dst -------- */
        if (t.type == TOK_IDENT) {
            if (mode == CALL_POSITIONAL)
                parse_error(P, t.line,
                            "cannot mix positional and named arguments");

            mode = CALL_NAMED;

            Token arrow = ts_next(P->stream);

            if (arrow.type == TOK_LESS) {
                if (seen_results)
                    parse_error(P, arrow.line, "parameter after result");

                Symbol* cp = lookup_symbol(callee, t.lexeme);
                if (!cp || cp->kind != SYM_PARAM)
                    parse_error(P, t.line,
                                "'%s' is not a parameter of '%s'",
                                t.lexeme, callee);
                parse_call_param(P, cp->offset);
            } else if (arrow.type == TOK_GREATER) {
                seen_results = 1;

                /* Run the callee before reading any result. */
                if (!exec_emitted) {
                    emit_call_exec(callee);
                    exec_emitted = 1;
                }

                Symbol* cr = lookup_symbol(callee, t.lexeme);
                if (!cr || cr->kind != SYM_RETURN)
                    parse_error(P, t.line,
                                "'%s' is not a result of '%s'",
                                t.lexeme, callee);

                Token dst = ts_next(P->stream);
                if (dst.type != TOK_IDENT)
                    parse_error(P, dst.line,
                                "expected identifier after '>'");
                Symbol* ds = get_or_create_receiver(P, dst.lexeme, dst.line);
                emit_call_result(ds->offset, cr->offset);
            } else {
                parse_error(P, arrow.line,
                            "expected '<' or '>' after '%s'", t.lexeme);
            }
            continue;
        }

        parse_error(P, t.line,
                    "unexpected token type %d in call arguments", (int)t.type);
    }

    /* If there were no results, the call has not been emitted yet. */
    if (!exec_emitted)
        emit_call_exec(callee);
}

/* ============================================================================
 * Statement dispatcher
 * ============================================================================
 *
 * This is the only public entry point. It dispatches on the current token:
 *
 *   '!'      → break
 *   '>'      → early return
 *   '@'      → indirect assignment
 *   IDENT+?  → branch (greedy binding)
 *   IDENT+{  → loop   (greedy binding)
 *   IDENT    → call or assignment, resolved via the symbol table
 *
 * Anything else is a hard error.
 *
 * The n parameter is the indent at which this statement must start. It is
 * passed by the caller (driver: 0; body of branch/loop: n+1) and used to
 * produce a precise error message if the token is at a different indent.
 */

void parse_statement(CodeParser* P, uint32_t n) {
    Token* t = ts_peek(P->stream);

    if (t->type == TOK_EOF)
        return;
    if (t->type == TOK_ERROR)
        parse_error(P, t->line, "lexical error");

    if (t->indent != n)
        parse_error(P, t->line,
                    "expected indent %u, got %u", n, t->indent);

    /* ----- '!' : break ------------------------------------------------ */
    if (t->type == TOK_BANG) {
        ts_next(P->stream);
        emit_break();
        return;
    }

    /* ----- '>' : early return ----------------------------------------- */
    if (t->type == TOK_GREATER) {
        ts_next(P->stream);
        emit_return();
        return;
    }

    /* ----- '@' : indirect assignment ----------------------------------- */
    if (t->type == TOK_AT) {
        parse_deref_assign(P, n);
        return;
    }

    /* ----- IDENT: branch, loop, call, or assignment ------------------- */
    if (t->type == TOK_IDENT) {
        Token* nxt = ts_peek2(P->stream);

        int next_at_same_indent = (nxt->indent == n && nxt->is_bol);

        /* Diagnose the classic "blank line between condition and operator"
         * case, which would otherwise produce a confusing "invalid source"
         * error later. */
        if (next_at_same_indent &&
            (nxt->type == TOK_QUESTION || nxt->type == TOK_LBRACE) &&
            nxt->is_after_blank) {
            parse_error(P, nxt->line,
                        "blank line between a condition and '%s' is not "
                        "allowed; remove the blank line",
                        nxt->type == TOK_QUESTION ? "?" : "{");
        }

        /* Greedy binding: '?' or '{' on the immediately following line
         * (no blank line, no other tokens in between). Comments are fine. */
        int glued = next_at_same_indent && !nxt->is_after_blank;

        if (glued && nxt->type == TOK_QUESTION) {
            parse_branch(P, n);
            return;
        }
        if (glued && nxt->type == TOK_LBRACE) {
            parse_loop(P, n);
            return;
        }

        /* Not a branch/loop: look up the name. */
        Symbol* s = lookup_symbol(P->module_path, t->lexeme);

        if (s && s->kind == SYM_ALIAS) {
            uint32_t call_line = t->line;
            ts_next(P->stream);
            parse_call(P, n, s, call_line);
            return;
        }

        /* Assignment (possibly declaring a new local). */
        Symbol* dst = get_or_create_receiver(P, t->lexeme, t->line);
        ts_next(P->stream);
        parse_assign(P, n, dst->offset);
        return;
    }

    parse_error(P, t->line, "unexpected token type %d at statement start",
                (int)t->type);
}