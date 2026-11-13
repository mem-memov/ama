/**
 * @file
 * @brief Append ordered transfer/selection actions without consuming selector result ordinals.
 * [CALL-09](../../../spec/language/calls-and-results.md#call-09) makes each
 * complete selected subtree precede its next sibling while keeping binding style independent.
 */
#include "../internal.h"
bool parser_output_action(Parser *parser, AstNode *owner, AstArgument *binding, AstNode *operation) {
    AstOutputAction *action = parser_allocate(parser, sizeof(*action), parser_site(parser));
    if (action == NULL) { return false; }
    *action = (AstOutputAction){.kind = operation == NULL ? AST_BINDING_ACTION : AST_SELECTED_ACTION,
        .binding = binding, .operation = operation};
    if (owner->as.call.last_output == NULL) { owner->as.call.first_output = action; }
    else { owner->as.call.last_output->next = action; }
    owner->as.call.last_output = action;
    owner->as.call.outputs_started = true;
    if (operation != NULL) { owner->as.call.has_selected = true; }
    return true;
}
