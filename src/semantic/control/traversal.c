/** @file
 * @brief Retain the exact lexical collection/binder role already classified by source-order inference.
 * [FLOW-05](../../../spec/language/lists.md#flow-05) distinguishes traversal
 * from indexed truth testing and never creates a persistent iterator field.
 */
#include "internal.h"
/** Reuse the source-order inference role by exact foreach syntax identity; no new runtime binding or module field is manufactured. */
bool semantic_control_traversal(SemanticProgram *program, SemanticType *type, SemanticStatement *statement) {
    for (const SemanticForeachBinding *role = type->foreach_bindings; role != NULL; role = role->next) {
        if (role->syntax == statement->syntax) { statement->traversal = role; return true; }
    }
    return semantic_fail(program, statement->syntax->span, "The traversal has no resolved lexical binding", NULL);
}
