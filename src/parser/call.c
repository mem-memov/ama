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

static bool resolve_callee(CodeParser* P, CallState* call,
                           const Symbol* alias, uint32_t line) {
    if (!normalize_path(alias->path_link, call->callee, P->error)) return false;
    if (!graph_has_node(P->graph, call->callee))
        return parser_error(P, line,
                 "module '%s' not found (called from '%s')",
                 call->callee, P->module_path);
    return true;
}

/* Execution happens once, before reading any result, even for an empty call. */
static void ensure_call_emitted(CodeParser* P, CallState* call) {
    if (!call->exec_emitted) {
        emit_call_exec(P->emitter, call->callee);
        call->exec_emitted = 1;
    }
}

static bool set_call_mode(CodeParser* P, CallState* call,
                          CallMode mode, uint32_t line) {
    if (call->mode != CALL_INIT && call->mode != mode)
        return parser_error(P, line, "cannot mix positional and named arguments");
    call->mode = mode;
    return true;
}

static bool parse_parameter(CodeParser* P, CallState* call,
                            const Symbol* param, uint32_t line) {
    if (call->seen_results)
        return parser_error(P, line, "parameter after result");
    return parse_call_param(P, param->id);
}

static bool parse_result(CodeParser* P, CallState* call, const Symbol* result) {
    call->seen_results = 1;
    ensure_call_emitted(P, call);
    Token dst = ts_next(P->stream);
    if (dst.type != TOK_IDENT)
        return parser_error(P, dst.line, "expected identifier after '>'");
    Symbol* receiver = get_or_create_receiver(P, dst.lexeme, dst.line);
    if (!receiver) return false;
    emit_copy(P->emitter, receiver->id, result->id);
    return true;
}

static bool parse_positional_argument(CodeParser* P, CallState* call, Token arrow) {
    if (!set_call_mode(P, call, CALL_POSITIONAL, arrow.line)) return false;
    int is_param = arrow.type == TOK_LESS;
    if (is_param && call->seen_results)
        return parser_error(P, arrow.line, "parameter after result");
    int index = is_param ? call->param_index++ : call->result_index++;
    Symbol* symbol = get_nth_symbol(P->symbols, call->callee, index,
                                   is_param ? SYM_PARAM : SYM_RETURN);
    if (!symbol)
        return parser_error(P, arrow.line, "too many positional %s for '%s'",
                 is_param ? "parameters" : "results", call->callee);
    if (is_param)
        return parse_parameter(P, call, symbol, arrow.line);
    else
        return parse_result(P, call, symbol);
}

static bool parse_named_argument(CodeParser* P, CallState* call, Token name) {
    if (!set_call_mode(P, call, CALL_NAMED, name.line)) return false;
    Token arrow = ts_next(P->stream);
    if (arrow.type != TOK_LESS && arrow.type != TOK_GREATER)
        return parser_error(P, arrow.line, "expected '<' or '>' after '%s'",
                 name.lexeme);
    int is_param = arrow.type == TOK_LESS;
    if (is_param && call->seen_results)
        return parser_error(P, arrow.line, "parameter after result");
    Symbol* symbol = lookup_symbol(P->symbols, call->callee, name.lexeme);
    if (!symbol || symbol->kind != (is_param ? SYM_PARAM : SYM_RETURN))
        return parser_error(P, name.line, "'%s' is not a %s of '%s'",
                 name.lexeme, is_param ? "parameter" : "result", call->callee);
    if (is_param)
        return parse_parameter(P, call, symbol, arrow.line);
    else
        return parse_result(P, call, symbol);
}

static bool parse_call_argument(CodeParser* P, CallState* call) {
    Token token = ts_next(P->stream);
    switch (token.type) {
    case TOK_LESS:
    case TOK_GREATER:
        return parse_positional_argument(P, call, token);
    case TOK_IDENT:
        return parse_named_argument(P, call, token);
    default:
        return parser_error(P, token.line,
                 "unexpected token type %d in call arguments", (int)token.type);
    }
}

bool parse_call(CodeParser* P, uint32_t indent, Symbol* alias, uint32_t line) {
    CallState call = {0};
    if (!resolve_callee(P, &call, alias, line)) return false;
    emit_call_init(P->emitter, call.callee);
    while (ts_peek(P->stream)->type != TOK_EOF &&
           ts_peek(P->stream)->type != TOK_ERROR &&
           ts_peek(P->stream)->indent == indent + 1)
        if (!parse_call_argument(P, &call)) return false;
    ensure_call_emitted(P, &call);
    return true;
}
