#include "parser/internal.h"
#include "common/path.h"
#include "graph/graph.h"

typedef enum { CALL_ARGUMENTS_UNSET, CALL_ARGUMENTS_POSITIONAL, CALL_ARGUMENTS_NAMED } CallArgumentMode;

typedef struct {
    char callee[MAX_MODULE_PATH_LENGTH];
    SourceLocation location;
    CallArgumentMode mode;
    int has_results;
    int has_emitted_call;
    int parameter_index;
    int result_index;
} ModuleCallState;

static bool resolve_callee(StatementParser* parser, ModuleCallState* call,
    const Symbol* alias, uint32_t line_number) {
    if (!normalize_path(alias->alias_path, call->callee, parser->error)) return false;
    if (!graph_has_node(parser->graph, call->callee))
        return parser_error(parser, line_number,
            "module '%s' not found (called from '%s')",
            call->callee, parser->module_path);
    return true;
}

/* Execution happens once, before reading any result, even for an empty call. */
static void ensure_call_emitted(StatementParser* parser, ModuleCallState* call) {
    if (!call->has_emitted_call) {
        emit_source_location(parser->emitter, call->location);
        emit_call_exec(parser->emitter, call->callee);
        call->has_emitted_call = 1;
    }
}

static bool set_call_mode(StatementParser* parser, ModuleCallState* call,
    CallArgumentMode mode, uint32_t line_number) {
    if (call->mode != CALL_ARGUMENTS_UNSET && call->mode != mode)
        return parser_error(parser, line_number, "cannot mix positional and named arguments");
    call->mode = mode;
    return true;
}

static bool parse_parameter(StatementParser* parser, ModuleCallState* call,
    const Symbol* parameter, uint32_t line_number) {
    if (call->has_results)
        return parser_error(parser, line_number, "parameter after result");
    return parse_call_parameter(parser, parameter->symbol_id);
}

static bool parse_result(StatementParser* parser, ModuleCallState* call, const Symbol* result) {
    call->has_results = 1;
    ensure_call_emitted(parser, call);
    Token destination_token = token_stream_advance(parser->stream);
    if (destination_token.type != TOKEN_IDENTIFIER)
        return parser_error(parser, destination_token.line_number, "expected identifier after '>'");
    Symbol* receiver = get_or_create_receiver(parser, destination_token.lexeme, destination_token.line_number);
    if (!receiver) return false;
    emit_source_location(parser->emitter, token_location(destination_token));
    emit_copy(parser->emitter, receiver->symbol_id, result->symbol_id);
    return true;
}

static bool parse_positional_argument(StatementParser* parser, ModuleCallState* call, Token arrow) {
    if (!set_call_mode(parser, call, CALL_ARGUMENTS_POSITIONAL, arrow.line_number)) return false;
    int is_parameter = arrow.type == TOKEN_PARAMETER_ARROW;
    if (is_parameter && call->has_results)
        return parser_error(parser, arrow.line_number, "parameter after result");
    int index = is_parameter ? call->parameter_index++ : call->result_index++;
    Symbol* symbol = find_symbol_by_position(parser->symbols, call->callee, index,
        is_parameter ? SYMBOL_PARAMETER : SYMBOL_RESULT);
    if (!symbol)
        return parser_error(parser, arrow.line_number, "too many positional %s for '%s'",
            is_parameter ? "parameters" : "results", call->callee);
    if (is_parameter)
        return parse_parameter(parser, call, symbol, arrow.line_number);
    else
        return parse_result(parser, call, symbol);
}

static bool parse_named_argument(StatementParser* parser, ModuleCallState* call, Token name) {
    if (!set_call_mode(parser, call, CALL_ARGUMENTS_NAMED, name.line_number)) return false;
    Token arrow = token_stream_advance(parser->stream);
    if (arrow.type != TOKEN_PARAMETER_ARROW && arrow.type != TOKEN_RESULT_ARROW)
        return parser_error(parser, arrow.line_number, "expected '<' or '>' after '%s'",
            name.lexeme);
    int is_parameter = arrow.type == TOKEN_PARAMETER_ARROW;
    if (is_parameter && call->has_results)
        return parser_error(parser, arrow.line_number, "parameter after result");
    Symbol* symbol = lookup_symbol(parser->symbols, call->callee, name.lexeme);
    if (!symbol || symbol->kind != (is_parameter ? SYMBOL_PARAMETER : SYMBOL_RESULT))
        return parser_error(parser, name.line_number, "'%s' is not a %s of '%s'",
            name.lexeme, is_parameter ? "parameter" : "result", call->callee);
    if (is_parameter)
        return parse_parameter(parser, call, symbol, arrow.line_number);
    else
        return parse_result(parser, call, symbol);
}

static bool parse_call_argument(StatementParser* parser, ModuleCallState* call) {
    Token token = token_stream_advance(parser->stream);
    switch (token.type) {
    case TOKEN_PARAMETER_ARROW:
    case TOKEN_RESULT_ARROW:
        return parse_positional_argument(parser, call, token);
    case TOKEN_IDENTIFIER:
        return parse_named_argument(parser, call, token);
    default:
        return parser_error(parser, token.line_number,
            "unexpected token type %d in call arguments", (int)token.type);
    }
}

bool parse_call(StatementParser* parser, uint32_t indentation, Symbol* alias, SourceLocation location) {
    ModuleCallState call = {.location = location};
    if (!resolve_callee(parser, &call, alias, location.line)) return false;
    emit_call_init(parser->emitter, call.callee);
    while (token_stream_peek(parser->stream)->type != TOKEN_END_OF_FILE &&
        token_stream_peek(parser->stream)->type != TOKEN_ERROR &&
        token_stream_peek(parser->stream)->indentation == indentation + 1)
        if (!parse_call_argument(parser, &call)) return false;
    ensure_call_emitted(parser, &call);
    return true;
}
