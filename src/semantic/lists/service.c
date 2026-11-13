/** @file
 * @brief Classify whole-list parents as intrinsic service scopes with no ordinary instance bindings.
 * [LIST-04](../../../spec/language/lists.md#list-04) requires an unmarked
 * mutable list parent and at least one selected method under its exact element type.
 */
#include "internal.h"
bool semantic_list_service(SemanticProgram *program, SemanticStatement *statement) {
    const AstNode *node = statement->syntax;
    if (node->as.call.mode != AST_RETAINED_READ || node->as.call.input_count != 0
        || node->as.call.output_count != 0 || !node->as.call.has_selected) {
        return semantic_fail(program, node->span, "A whole list service parent accepts only selected entry methods", NULL);
    }
    SemanticType *integer = semantic_list_integer(program, node->span);
    if (integer == NULL) { return false; }
    statement->operation_kind = SEMANTIC_LIST_SERVICE_OPERATION;
    statement->list_interface = (SemanticListInterface){statement->namespace_type, integer, statement->access->reference};
    statement->namespace_type = NULL;
    statement->list_service = statement;
    return true;
}
