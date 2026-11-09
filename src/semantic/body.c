#include "semantic/internal.h"

static bool collect_receivers(SemanticContext* context, AstBlock* block) {
    for (AstStatement* statement = block->first; statement; statement = statement->next) {
        switch (statement->kind) {
        case AST_DECLARATION:
            return semantic_error(context, statement->location, "typed declarations require the module-value ABI");
        case AST_ASSIGNMENT: {
            AstName* name = &statement->as.assignment.receiver;
            Symbol* symbol = resolve_receiver(context, name->text, name->location);
            if (!symbol) return false;
            name->symbol_id = symbol->symbol_id;
            break;
        }
        case AST_CALL:
            for (AstArgument* argument = statement->as.call.arguments; argument; argument = argument->next) {
                if (argument->kind == AST_PARAMETER_ARGUMENT) continue;
                Symbol* symbol = resolve_receiver(context, argument->value.text, argument->value.location);
                if (!symbol) return false;
                argument->value.symbol_id = symbol->symbol_id;
            }
            break;
        case AST_BRANCH:
        case AST_LOOP:
            if (!collect_receivers(context, &statement->as.control.body) ||
                !collect_receivers(context, &statement->as.control.alternative)) return false;
            break;
        case AST_BREAK:
        case AST_RETURN:
            break;
        }
    }
    return true;
}

static bool validate_block(SemanticContext* context, AstBlock* block, unsigned int loop_depth) {
    for (AstStatement* statement = block->first; statement; statement = statement->next) {
        switch (statement->kind) {
        case AST_DECLARATION:
            return semantic_error(context, statement->location, "typed declarations require the module-value ABI");
        case AST_ASSIGNMENT:
            if (!resolve_value(context, &statement->as.assignment.source)) return false;
            break;
        case AST_CALL:
            if (!validate_call(context, statement)) return false;
            break;
        case AST_BRANCH:
        case AST_LOOP:
            if (!resolve_value(context, &statement->as.control.condition) ||
                !validate_block(context, &statement->as.control.body,
                    loop_depth + (statement->kind == AST_LOOP ? 1 : 0)) ||
                !validate_block(context, &statement->as.control.alternative, loop_depth)) return false;
            break;
        case AST_BREAK:
            if (!loop_depth)
                return semantic_error(context, statement->location, "loop exit '^' is only allowed inside a loop");
            break;
        case AST_RETURN:
            break;
        }
    }
    return true;
}

bool validate_module_body(AstModule* module, SymbolTable* symbols,
    const Graph* graph, const char* module_path, Error* error) {
    module->validated = false;
    SemanticContext context = {symbols, graph, module_path, error};
    if (!collect_receivers(&context, &module->body) || !validate_block(&context, &module->body, 0)) return false;
    module->validated = true;
    return true;
}
