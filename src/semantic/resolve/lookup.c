/** @file
 * @brief Locate retained statements by exact syntax identity for operation parents, loop roots and lexical exits.
 * [CALL-09](../../../spec/language/calls-and-results.md#call-09) ties each
 * selector to its particular enclosing operation, including repeated selections.
 * [FLOW-04](../../../spec/language/control-flow.md#flow-04) retains exact
 * restart and incoming-loop identities rather than selecting a nearby call.
 */
#include "semantic/internal.h"
SemanticStatement *semantic_statement_find(SemanticType *type, const AstNode *node) {
    for (SemanticStatement *statement = type->statements; statement != NULL; statement = statement->next) {
        if (statement->syntax == node) { return statement; }
    }
    return NULL;
}
