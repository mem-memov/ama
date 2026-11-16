/** @file
 * @brief Link typed output bindings and selected operation subtrees in their original action order.
 * [CALL-09](../../../../spec/language/calls-and-results.md#call-09) requires
 * depth-first selected actions before later siblings, without duplicating bindings.
 */
#include "../internal.h"
/** Match an already checked binding by original argument identity; no name or positional reconstruction is permitted. Component contract: [CALL-09](../../../../spec/language/calls-and-results.md#call-09). */
static SemanticBinding *binding(SemanticStatement *statement, const AstArgument *syntax) {
    for (SemanticBinding *value = statement->bindings; value != NULL; value = value->next) {
        if (value->syntax == syntax) { return value; }
    }
    return NULL;
}
bool semantic_output_actions(SemanticProgram *program, SemanticType *type) {
    for (SemanticStatement *statement = type->statements; statement != NULL; statement = statement->next) {
        if (statement->syntax->kind != AST_CALL) { continue; }
        for (const AstOutputAction *syntax = statement->syntax->as.call.first_output; syntax != NULL; syntax = syntax->next) {
            SemanticOutputAction *action = semantic_allocate(program, sizeof(*action));
            if (action == NULL) { return false; }
            action->syntax = syntax;
            if (syntax->kind == AST_BINDING_ACTION) {
                action->binding = binding(statement, syntax->binding);
                if (action->binding == NULL) { return semantic_fail(program, statement->syntax->span, "The output binding has no resolved interface", NULL); }
            } else {
                action->operation = semantic_statement_find(type, syntax->operation);
                if (action->operation == NULL || action->operation->enclosing_operation != statement) {
                    return semantic_fail(program, statement->syntax->span, "The selected action has no resolved parent operation", NULL);
                }
            }
            if (statement->last_action == NULL) { statement->actions = action; }
            else { statement->last_action->next = action; }
            statement->last_action = action;
        }
    }
    return true;
}
