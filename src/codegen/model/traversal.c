/** @file
 * @brief Query exact lexical foreach identities independently of emission order.
 * [FLOW-05](../../../spec/language/lists.md#flow-05) fixes one scope per source
 * site and shares a binding only by its resolved symbol, never by spelling or
 * runtime contents. Ordinals enumerate foreach sites, excluding ordinary loops.
 */
#include "codegen/internal.h"

const PlannedLoop *codegen_foreach_binding(const ScaffoldType *type, const SemanticSymbol *symbol, size_t *output) {
    size_t index = 0;
    for (const PlannedLoop *loop = type->plan->loops; loop != NULL; loop = loop->next) {
        if (loop->statement->syntax->kind != AST_FOREACH) { continue; }
        if (loop->statement->traversal->binding == symbol) { *output = index; return loop; }
        index++;
    }
    return NULL;
}

bool codegen_foreach_scope_index(const ScaffoldType *type, const PlannedLoop *target, size_t *output) {
    size_t index = 0;
    for (const PlannedLoop *loop = type->plan->loops; loop != NULL; loop = loop->next) {
        if (loop->statement->syntax->kind != AST_FOREACH) { continue; }
        if (loop == target) { *output = index; return true; }
        index++;
    }
    return false;
}
