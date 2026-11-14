/** @file
 * @brief Reserve exact lexical scan slots and coalesce nested reused bindings.
 * [FLOW-05](../../../spec/language/lists.md#flow-05) gives each scan its own
 * boundary but reuses the same binding cell for a shared reference symbol.
 * [RT-02](../../../spec/implementation/runtime.md#rt-02) makes storage independent
 * of list length and excludes these cells from declared-field/native cloning.
 */
#include "codegen/internal.h"

bool codegen_emit_traversal_storage(FILE *output, const ScaffoldType *type) {
    for (const PlannedLoop *loop = type->plan->loops; loop != NULL; loop = loop->next) {
        if (loop->statement->syntax->kind != AST_FOREACH) { continue; }
        if (fprintf(output, "    AmaProgramForeachCursor foreach_cursor_%zu;\n", loop->entry->selector) < 0) { return false; }
        const SemanticSymbol *binding = loop->statement->traversal->binding;
        bool earlier = false;
        for (const PlannedLoop *prior = type->plan->loops; prior != loop; prior = prior->next) {
            if (prior->statement->syntax->kind == AST_FOREACH
                && prior->statement->traversal->binding == binding) { earlier = true; break; }
        }
        if (!earlier && fprintf(output, "    AmaProgramForeachBinding foreach_binding_%zu;\n", loop->entry->selector) < 0) { return false; }
    }
    return true;
}
