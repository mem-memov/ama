/**
 * @file
 * @brief Resolve ordinary and intrinsic typed bindings without hidden method instances.
 *
 * [LIST-02](../../../spec/language/lists.md#list-02) forbids whole-list execution.
 * [LIST-04](../../../spec/language/lists.md#list-04) gives each intrinsic method
 * its own typed interface, so no positional dispatcher binding may reach IR.
 * [CALL-09](../../../spec/language/calls-and-results.md#call-09) requires complete
 * operation-tree resolution: selectors retain parent fields while ordinary operands remain caller-side.
 */
#include "internal.h"
#include "semantic/lists/internal.h"

bool semantic_call(SemanticProgram *program, SemanticType *type, SemanticStatement *statement) {
    if (!semantic_call_target(program, type, statement)) {
        return false;
    }
    for (const AstArgument *argument = statement->syntax->as.call.first;
        argument != NULL; argument = argument->next) {
        SemanticBinding *binding = semantic_allocate(program, sizeof(*binding));
        if (binding == NULL) {
            return false;
        }
        binding->syntax = argument;
        if (!semantic_call_field(program, statement, argument, binding)
            || !semantic_binding_value(program, type, statement, binding)
            || !semantic_list_copy_requirement(program, statement, binding)) {
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
