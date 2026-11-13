/**
 * @file
 * @brief Resolve scalar call bindings and reject unsupported whole-list services.
 *
 * [LIST-02](../../../spec/language/lists.md#list-02) forbids whole-list execution.
 * [LIST-04](../../../spec/language/lists.md#list-04) gives each intrinsic method
 * its own typed interface, so no positional dispatcher binding may reach IR.
 * [CALL-09](../../../spec/language/calls-and-results.md#call-09) requires complete
 * operation-tree resolution: selectors retain parent fields while ordinary operands remain caller-side.
 */
#include "internal.h"

bool semantic_call(SemanticProgram *program, SemanticType *type, SemanticStatement *statement) {
    if (!semantic_call_target(program, type, statement)) {
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
