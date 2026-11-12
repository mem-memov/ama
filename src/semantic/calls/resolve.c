/**
 * @file
 * @brief Resolve scalar call bindings and reject unsupported whole-list services.
 *
 * [LIST-02](../../../spec/language/lists.md#list-02) forbids whole-list execution.
 * [LIST-04](../../../spec/language/lists.md#list-04) gives each intrinsic method
 * its own typed interface, so no positional dispatcher binding may reach IR.
 */
#include "internal.h"

/** Resolve the exact explicit constant namespace or scalar instance form; a constant symbol cannot become an executable instance target.
 */
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

bool semantic_call(SemanticProgram *program, SemanticType *type, SemanticStatement *statement) {
    if (!target(program, type, statement)) {
        return false;
    }
    bool list = statement->access != NULL && statement->access->list;
    if (list) {
        return semantic_fail(program, statement->syntax->span,
            "A whole list cannot execute or expose retained results; typed list services are not implemented", NULL);
    }
    for (const AstArgument *argument = statement->syntax->as.call.first;
        argument != NULL; argument = argument->next) {
        SemanticBinding *binding = semantic_allocate(program, sizeof(*binding));
        if (binding == NULL) {
            return false;
        }
        binding->syntax = argument;
        if (!semantic_call_field(program, statement, argument, binding)
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
