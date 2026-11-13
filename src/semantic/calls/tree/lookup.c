/** @file
 * @brief Locate operation parents by resolved syntax identity rather than source names or dynamic caller chains.
 * [CALL-09](../../../../spec/language/calls-and-results.md#call-09) ties each
 * selector to its particular enclosing operation, including repeated selections.
 */
#include "../internal.h"
SemanticStatement *semantic_call_statement(SemanticType *type, const AstNode *node) {
    for (SemanticStatement *statement = type->statements; statement != NULL; statement = statement->next) {
        if (statement->syntax == node) { return statement; }
    }
    return NULL;
}
