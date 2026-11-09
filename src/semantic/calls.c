#include "semantic/internal.h"
#include "common/path.h"

bool validate_call(SemanticContext* context, AstStatement* statement) {
    AstName* target = &statement->as.call.target;
    Symbol* alias = lookup_symbol(context->symbols, context->module_path, target->text);
    if (!alias || alias->kind != SYMBOL_MODULE_ALIAS)
        return semantic_error(context, target->location, "unknown module alias '%s'", target->text);
    target->symbol_id = alias->symbol_id;
    char* callee = statement->as.call.module_path;
    if (!normalize_path(alias->alias_path, callee, context->error)) return false;
    if (!graph_has_node(context->graph, callee))
        return semantic_error(context, target->location,
            "module '%s' not found (called from '%s')", callee, context->module_path);
    bool has_results = false;
    int mode = 0, parameter_index = 0, result_index = 0;
    for (AstArgument* argument = statement->as.call.arguments; argument; argument = argument->next) {
        int next_mode = argument->is_named ? 1 : 2;
        if (mode && mode != next_mode)
            return semantic_error(context, argument->location, "cannot mix positional and named arguments");
        mode = next_mode;
        if (argument->is_parameter && has_results)
            return semantic_error(context, argument->location, "parameter after result");
        SymbolKind kind = argument->is_parameter ? SYMBOL_PARAMETER : SYMBOL_RESULT;
        Symbol* symbol;
        if (argument->is_named) {
            symbol = lookup_symbol(context->symbols, callee, argument->label.text);
            if (!symbol || symbol->kind != kind)
                return semantic_error(context, argument->location, "'%s' is not a %s of '%s'",
                    argument->label.text, argument->is_parameter ? "parameter" : "result", callee);
            argument->label.symbol_id = symbol->symbol_id;
        } else {
            int position = argument->is_parameter ? parameter_index++ : result_index++;
            symbol = find_symbol_by_position(context->symbols, callee, position, kind);
            if (!symbol)
                return semantic_error(context, argument->location, "too many positional %s for '%s'",
                    argument->is_parameter ? "parameters" : "results", callee);
        }
        argument->callee_symbol_id = symbol->symbol_id;
        if (argument->is_parameter) {
            if (!resolve_value(context, &argument->value)) return false;
        } else {
            has_results = true;
            Symbol* receiver = resolve_receiver(context, argument->value.text, argument->value.location);
            if (!receiver) return false;
            argument->value.symbol_id = receiver->symbol_id;
        }
    }
    return true;
}
