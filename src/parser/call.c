#include "parser/internal.h"
#include "common/path.h"
#include "graph/graph.h"

typedef enum { CALL_INIT, CALL_POSITIONAL, CALL_NAMED } CallMode;

typedef struct {
    char callee[MAX_PATH_LEN];
    CallMode mode;
    int seen_results;
    int exec_emitted;
    int param_index;
    int result_index;
} CallState;

static void resolve_callee(CodeParser* P, CallState* call,
                           const Symbol* alias, uint32_t line) {
    normalize_path(alias->path_link, call->callee);
    if (!graph_has_node(call->callee))
        fatal_at(P->module_path, line,
                 "module '%s' not found (called from '%s')",
                 call->callee, P->module_path);
}

/* Execution happens once, before reading any result, even for an empty call. */
static void ensure_call_emitted(CallState* call) {
    if (!call->exec_emitted) {
        emit_call_exec(call->callee);
        call->exec_emitted = 1;
    }
}

static void set_call_mode(CodeParser* P, CallState* call,
                          CallMode mode, uint32_t line) {
    if (call->mode != CALL_INIT && call->mode != mode)
        fatal_at(P->module_path, line, "cannot mix positional and named arguments");
    call->mode = mode;
}

static void parse_parameter(CodeParser* P, CallState* call,
                            const Symbol* param, uint32_t line) {
    if (call->seen_results)
        fatal_at(P->module_path, line, "parameter after result");
    parse_call_param(P, param->id);
}

static void parse_result(CodeParser* P, CallState* call, const Symbol* result) {
    call->seen_results = 1;
    ensure_call_emitted(call);
    Token dst = ts_next(P->stream);
    if (dst.type != TOK_IDENT)
        fatal_at(P->module_path, dst.line, "expected identifier after '>'");
    Symbol* receiver = get_or_create_receiver(P, dst.lexeme, dst.line);
    emit_copy(receiver->id, result->id);
}

static void parse_positional_argument(CodeParser* P, CallState* call, Token arrow) {
    set_call_mode(P, call, CALL_POSITIONAL, arrow.line);
    int is_param = arrow.type == TOK_LESS;
    if (is_param && call->seen_results)
        fatal_at(P->module_path, arrow.line, "parameter after result");
    int index = is_param ? call->param_index++ : call->result_index++;
    Symbol* symbol = get_nth_symbol(call->callee, index,
                                   is_param ? SYM_PARAM : SYM_RETURN);
    if (!symbol)
        fatal_at(P->module_path, arrow.line, "too many positional %s for '%s'",
                 is_param ? "parameters" : "results", call->callee);
    if (is_param)
        parse_parameter(P, call, symbol, arrow.line);
    else
        parse_result(P, call, symbol);
}

static void parse_named_argument(CodeParser* P, CallState* call, Token name) {
    set_call_mode(P, call, CALL_NAMED, name.line);
    Token arrow = ts_next(P->stream);
    if (arrow.type != TOK_LESS && arrow.type != TOK_GREATER)
        fatal_at(P->module_path, arrow.line, "expected '<' or '>' after '%s'",
                 name.lexeme);
    int is_param = arrow.type == TOK_LESS;
    if (is_param && call->seen_results)
        fatal_at(P->module_path, arrow.line, "parameter after result");
    Symbol* symbol = lookup_symbol(call->callee, name.lexeme);
    if (!symbol || symbol->kind != (is_param ? SYM_PARAM : SYM_RETURN))
        fatal_at(P->module_path, name.line, "'%s' is not a %s of '%s'",
                 name.lexeme, is_param ? "parameter" : "result", call->callee);
    if (is_param)
        parse_parameter(P, call, symbol, arrow.line);
    else
        parse_result(P, call, symbol);
}

static void parse_call_argument(CodeParser* P, CallState* call) {
    Token token = ts_next(P->stream);
    switch (token.type) {
    case TOK_LESS:
    case TOK_GREATER:
        parse_positional_argument(P, call, token);
        return;
    case TOK_IDENT:
        parse_named_argument(P, call, token);
        return;
    default:
        fatal_at(P->module_path, token.line,
                 "unexpected token type %d in call arguments", (int)token.type);
    }
}

void parse_call(CodeParser* P, uint32_t indent, Symbol* alias, uint32_t line) {
    CallState call = {0};
    resolve_callee(P, &call, alias, line);
    emit_call_init(call.callee);
    while (ts_peek(P->stream)->type != TOK_EOF &&
           ts_peek(P->stream)->type != TOK_ERROR &&
           ts_peek(P->stream)->indent == indent + 1)
        parse_call_argument(P, &call);
    ensure_call_emitted(&call);
}
