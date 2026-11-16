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

/** Select cursor sites by exact resolved collection-symbol identity. Component contract: [FLOW-05](../../../spec/language/lists.md#flow-05), [RT-02](../../../spec/implementation/runtime.md#rt-02). */
static bool codegen_cursor_field(const PlannedLoop *loop, const ScaffoldMember *member) {
    return loop->statement->syntax->kind == AST_FOREACH
        && loop->statement->traversal->collection == member->field->symbol;
}

bool codegen_emit_traversal_descriptors(FILE *output, const ScaffoldType *type) {
    if (type->plan->type->foreach_bindings == NULL) { return true; }
    const char *name = type->names[SCAFFOLD_WRITE]->internal_name;
    for (const ScaffoldMember *member = type->members; member != NULL; member = member->next) {
        bool opened = false;
        for (const PlannedLoop *loop = type->plan->loops; loop != NULL; loop = loop->next) {
            if (!codegen_cursor_field(loop, member)) { continue; }
            if (!opened && fprintf(output, "static const size_t %s_Cursors_%zu[] = {\n", name, member->field->slot) < 0) { return false; }
            opened = true;
            if (fprintf(output, "offsetof(%s_Storage,foreach_cursor_%zu),\n", name, loop->entry->selector) < 0) { return false; }
        }
        if (opened && fputs("};\n", output) == EOF) { return false; }
    }
    if (fprintf(output, "static const AmaListCursorSet %s_ListCursors[] = {\n", name) < 0) { return false; }
    for (const ScaffoldMember *member = type->members; member != NULL; member = member->next) {
        size_t count = 0;
        for (const PlannedLoop *loop = type->plan->loops; loop != NULL; loop = loop->next) {
            if (codegen_cursor_field(loop, member)) { count++; }
        }
        if (count == 0) {
            if (fputs("{0},\n", output) == EOF) { return false; }
        } else if (fprintf(output, "{.offsets=%s_Cursors_%zu,.count=%zu},\n", name, member->field->slot, count) < 0) { return false; }
    }
    return fputs("};\n", output) != EOF;
}
