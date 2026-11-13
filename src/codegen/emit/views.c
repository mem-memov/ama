/**
 * @file
 * @brief Emit typed operation-scoped native views bound to stable runtime storage.
 *
 * [CABI-03](../../../spec/implementation/native-interface.md#cabi-03)
 * [CABI-02](../../../spec/implementation/native-interface.md#cabi-02) checks
 * the expected nominal type before applying its C payload layout.
 * Owned layouts follow the checked eager dependency order; references expose declared storage without acquiring their targets.
 * [STACK-04](../../../spec/implementation/compiler.md#stack-04) permits ordinary
 * generated C helpers to use the stack. View binding is not Ama body dispatch;
 * only the native entry boundary carries mandatory inline authority.
 */
#include "codegen/internal.h"

/**
 * Map validated nonowned storage form to its reference/list view family; the owned case is
 * handled separately by callers.
 */
static ScaffoldTypeRole codegen_view_role(StorageForm form) {
    return form == STORAGE_REFERENCE ? SCAFFOLD_REFERENCE_STORAGE : form == STORAGE_OWNING_LIST
        ? SCAFFOLD_LIST_STORAGE : SCAFFOLD_REFERENCE_LIST_STORAGE;
}

/**
 * Emit operation-local read/write scope plus owned child scopes or reference/list storage views.
 * Checked ownership layout ordering ensures complete child C types.
 */
static bool codegen_view_type(NativeScaffold *scaffold, FILE *output, const ScaffoldType *type) {
    const char *name = type->names[SCAFFOLD_WRITE]->internal_name;
    if (fprintf(output, "typedef struct %s_Scope { %s read; %s write;\n", name,
            type->names[SCAFFOLD_READ]->internal_name, name) < 0) { return false; }
    for (const ScaffoldMember *member = type->members; member != NULL; member = member->next) {
        const ScaffoldType *child = native_scaffold_type(scaffold, member->field->symbol->type);
        if (member->field->form == STORAGE_OWNED) {
            if (fprintf(output, "%s_Scope field_%zu;\n", child->names[SCAFFOLD_WRITE]->internal_name, member->field->slot) < 0) { return false; }
        } else {
            ScaffoldTypeRole role = codegen_view_role(member->field->form);
            if (fprintf(output, "%s field_%zu_storage; %s field_%zu_read; %s field_%zu_write;\n",
                    child->names[role]->internal_name, member->field->slot, child->names[role + 1]->internal_name,
                    member->field->slot, child->names[role + 2]->internal_name, member->field->slot) < 0) { return false; }
        }
    }
    return fprintf(output, "} %s_Scope;\n", name) >= 0;
}

/**
 * Emit a binder that validates the local handle and borrows active stable storage. General binds
 * require nominal type and read eligibility; interface binds skip state fields and use the exact running receiver's invocation
 * authority. These ordinary static helpers may be optimized normally; they have
 * no mandatory inline boundary. Unused per-type binders remain harmless when no
 * native body needs that view.
 */
static bool codegen_view_bind(NativeScaffold *scaffold, FILE *output, const ScaffoldType *type, bool interface) {
    const char *name = type->names[SCAFFOLD_WRITE]->internal_name;
    if (fprintf(output, "static __attribute__((unused)) AmaStatus %s_%s(AmaNativeBorrowScope *borrow, AmaHandle handle, %s_Scope *scope) {\n"
            "AmaPoolCell *cell=NULL; AmaStatus status=ama_native_borrow_cell(borrow,handle,%s_Type.pool.type_id,%s,&cell);\n"
            "if(status.code!=AMA_OK) return status;\n"
            "%s_Storage *storage=ama_pool_payload(&%s_Type.pool,cell);\n(void)storage; (void)scope;\n", name, interface ? "BindInterface" : "Bind", name,
            name, interface ? "AMA_NATIVE_BORROW_RECEIVER" : "AMA_NATIVE_BORROW_READ", name, name) < 0) { return false; }
    if (type->plan->type->module->syntax.sections.kind == MODULE_NATIVE
        && fputs("scope->read.payload=&storage->payload; scope->write.payload=&storage->payload;\n", output) == EOF) { return false; }
    for (const ScaffoldMember *member = type->members; member != NULL; member = member->next) {
        if (interface && member->field->symbol->role == SYMBOL_STATE) { continue; }
        const ScaffoldType *child = native_scaffold_type(scaffold, member->field->symbol->type);
        size_t slot = member->field->slot;
        const char *field = member->identity->internal_name;
        if (member->field->form == STORAGE_OWNED) {
            if (fprintf(output, "status=%s_Bind(borrow,storage->%s,&scope->field_%zu);\n"
                    "if(status.code!=AMA_OK) return status;\n"
                    "scope->read.%s=&scope->field_%zu.read; scope->write.%s=&scope->field_%zu.write;\n",
                    child->names[SCAFFOLD_WRITE]->internal_name, field, slot, field, slot, field, slot) < 0) { return false; }
        } else if (fprintf(output, "scope->field_%zu_storage.location=&storage->%s;\n"
                "scope->field_%zu_read.storage=&scope->field_%zu_storage; scope->field_%zu_write.storage=&scope->field_%zu_storage;\n"
                "scope->read.%s=&scope->field_%zu_read; scope->write.%s=&scope->field_%zu_write;\n",
                slot, field, slot, slot, slot, slot, field, slot, field, slot) < 0) { return false; }
    }
    return fputs("return status;\n}\n", output) != EOF;
}

/**
 * Emit opaque declared reference/list locations, then scopes/binders in exact eager-layout
 * ordinal order. Reference cycles do not recursively expand storage.
 */
bool codegen_emit_views(NativeScaffold *scaffold, FILE *output) {
    if (fputs("#include \"runtime/program/native/borrow.h\"\n", output) == EOF) { return false; }
    size_t count = 0;
    for (const ScaffoldType *type = scaffold->types; type != NULL; type = type->next) {
        count++;
        for (ScaffoldTypeRole role = SCAFFOLD_REFERENCE_STORAGE; role <= SCAFFOLD_REFERENCE_LIST_STORAGE; role += 3) {
            if (fprintf(output, "struct %s { %s *location; };\n", type->names[role]->internal_name,
                    role == SCAFFOLD_REFERENCE_STORAGE ? "AmaHandle" : "AmaHandleList") < 0) { return false; }
        }
    }
    for (size_t ordinal = 0; ordinal < count; ordinal++) {
        for (const ScaffoldType *type = scaffold->types; type != NULL; type = type->next) {
            if (type->plan->requires_pool && type->plan->layout_ordinal == ordinal
                && (!codegen_view_type(scaffold, output, type) || !codegen_view_bind(scaffold, output, type, false)
                    || (type->plan->type->module->syntax.sections.kind == MODULE_NATIVE && !codegen_view_bind(scaffold, output, type, true)))) { return false; }
        }
    }
    return true;
}
