#include "internal.h"

static bool target(SemanticProgram *program, SemanticType *caller, SemanticStatement *statement) {
    const AstNode *node = statement->syntax;
    if (node->as.call.mode == AST_CONSTANT_READ) {
        statement->namespace_type = semantic_type(program, node->as.call.namespace_path);
        return statement->namespace_type != NULL || semantic_fail(program, node->span,
            "The constant namespace is unavailable", NULL);
    }
    statement->access = semantic_access(program, caller, node->as.call.target, false, NULL);
    if (statement->access == NULL) {
        return false;
    }
    if (statement->access->symbol->role == SYMBOL_CONSTANT) {
        return semantic_fail(program, node->as.call.target->span,
            "Shared constants cannot be executable or retained-result targets", statement->access->symbol);
    }
    statement->namespace_type = statement->access->symbol->type;
    return true;
}

static bool list_contract(SemanticProgram *program, SemanticStatement *statement) {
    for (size_t index = 0; index < 4; index++) {
        SemanticType *type = program->library_types[index];
        if (type == NULL || !type->cloneable || (index == 1 && !type->abi.bindings[ABI_INDEX].present)
            || (index == 2 && !type->abi.bindings[ABI_CONDITION].present)) {
            return semantic_fail(program, statement->syntax->span,
                "Whole-list operations require configured copyable action/index/condition/outcome types", NULL);
        }
    }
    return true;
}

bool semantic_call(SemanticProgram *program, SemanticType *type, SemanticStatement *statement) {
    if (!target(program, type, statement)) {
        return false;
    }
    bool list = statement->access != NULL && statement->access->list;
    if (list && !list_contract(program, statement)) {
        return false;
    }
    for (const AstArgument *argument = statement->syntax->as.call.first;
        argument != NULL; argument = argument->next) {
        SemanticBinding *binding = semantic_allocate(program, sizeof(*binding));
        if (binding == NULL) {
            return false;
        }
        binding->syntax = argument;
        if (!(list ? semantic_list_field(program, statement, argument, binding)
                : semantic_call_field(program, statement, argument, binding))
            || !semantic_binding_value(program, type, statement, binding)) {
            return false;
        }
        if (statement->last_binding == NULL) {
            statement->bindings = binding;
        } else {
            statement->last_binding->next = binding;
        }
        statement->last_binding = binding;
    }
    return true;
}
