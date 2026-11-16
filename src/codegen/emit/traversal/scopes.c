/** @file
 * @brief Emit exact lexical scope ancestry and shared binding access descriptors.
 * [FLOW-05](../../../../spec/language/lists.md#flow-05) validates outer scopes
 * first and reuses reference symbols without shadowing or name-based heuristics.
 * [RT-02](../../../../spec/implementation/runtime.md#rt-02) derives every cursor,
 * binding and declared reference location from actual C offsetof expressions.
 */
#include "codegen/internal.h"

/** Require an actual known loop identity; SIZE_MAX denotes an absent root parent. Component contract: [FLOW-05](../../../../spec/language/lists.md#flow-05), [RT-02](../../../../spec/implementation/runtime.md#rt-02). */
static bool codegen_foreach_parent(const ScaffoldType *type, const PlannedLoop *loop, size_t *output) {
    const PlannedLoop *parent = loop->parent;
    while (parent != NULL && parent->statement->syntax->kind != AST_FOREACH) { parent = parent->parent; }
    if (parent == NULL) { *output = SIZE_MAX; return true; }
    return codegen_foreach_scope_index(type, parent, output);
}

/** Emit one typed plan; nominal member identity, not user spelling, selects offsets. Component contract: [FLOW-05](../../../../spec/language/lists.md#flow-05), [RT-02](../../../../spec/implementation/runtime.md#rt-02). */
static bool codegen_foreach_plan(NativeScaffold *scaffold, FILE *output,
    const ScaffoldType *type, const PlannedLoop *loop) {
    const SemanticForeachBinding *role = loop->statement->traversal;
    const ScaffoldMember *collection = codegen_member(type, role->collection);
    const ScaffoldType *element = native_scaffold_type(scaffold, role->binding->type);
    size_t unused = 0;
    const PlannedLoop *first = codegen_foreach_binding(type, role->binding, &unused);
    if (collection == NULL || element == NULL || first == NULL) {
        return scaffold_error(scaffold, DIAGNOSTIC_TYPE_MISMATCH, "A traversal has no exact resolved storage role", NULL);
    }
    const char *name = type->names[SCAFFOLD_WRITE]->internal_name;
    if (fprintf(output, ".plan={.field=%zu,.element_type=UINT64_C(%zu),.local=%s,"
        ".cursor_offset=offsetof(%s_Storage,foreach_cursor_%zu),"
        ".binding_offset=offsetof(%s_Storage,foreach_binding_%zu),.value_offset=",
        collection->field->slot, codegen_type_id(scaffold, element), role->binding->local ? "true" : "false",
        name, loop->entry->selector, name, first->entry->selector) < 0) { return false; }
    if (role->binding->local) {
        if (fprintf(output, "offsetof(%s_Storage,foreach_binding_%zu)+offsetof(AmaProgramForeachBinding,local_value)",
            name, first->entry->selector) < 0) { return false; }
    } else {
        const ScaffoldMember *binding = codegen_member(type, role->binding);
        if (binding == NULL) { return scaffold_error(scaffold, DIAGNOSTIC_TYPE_MISMATCH, "A reused traversal reference has no declared field", NULL); }
        if (fprintf(output, "offsetof(%s_Storage,%s)", name, binding->identity->internal_name) < 0) { return false; }
    }
    return fputs("}", output) != EOF;
}

bool codegen_emit_traversal_scopes(NativeScaffold *scaffold, FILE *output, const ScaffoldType *type) {
    if (type->plan->type->foreach_bindings == NULL) { return true; }
    const char *name = type->names[SCAFFOLD_WRITE]->internal_name;
    if (fprintf(output, "static const AmaProgramForeachScope %s_ForeachScopes[] = {\n", name) < 0) { return false; }
    for (const PlannedLoop *loop = type->plan->loops; loop != NULL; loop = loop->next) {
        if (loop->statement->syntax->kind != AST_FOREACH) { continue; }
        if (fputs("{", output) == EOF || !codegen_foreach_plan(scaffold, output, type, loop)) { return false; }
        size_t parent;
        if (!codegen_foreach_parent(type, loop, &parent)) {
            return scaffold_error(scaffold, DIAGNOSTIC_TYPE_MISMATCH, "A traversal parent has no exact lexical scope index", NULL);
        }
        if (parent == SIZE_MAX ? fputs(",.parent=SIZE_MAX", output) == EOF : fprintf(output, ",.parent=%zu", parent) < 0) { return false; }
        if (fprintf(output, ",.header=UINT64_C(%zu),.repeat=UINT64_C(%zu)},\n", loop->entry->selector, loop->repeat->selector) < 0) { return false; }
    }
    return fputs("};\n", output) != EOF;
}
