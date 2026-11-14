/**
 * @file
 * @brief Emit declared handle/list fields followed by actual native payload storage.
 *
 * [RT-02](../../../spec/implementation/runtime.md#rt-02)
 * Types without pools allocate no instance storage. Mutable type descriptors belong to the owning runtime context.
 */
#include "codegen/internal.h"

/**
 * Write one C storage structure per pool-backed type: declared handles/lists followed by actual
 * native State or a reserved Ama payload byte. No default instance is emitted.
 */
bool codegen_emit_storage(NativeScaffold *scaffold, FILE *output) {
    for (const ScaffoldType *type = scaffold->types; type != NULL; type = type->next) {
        if (!type->plan->requires_pool) { continue; }
        const char *name = type->names[SCAFFOLD_WRITE]->internal_name;
        if (fprintf(output, "typedef struct %s_Storage {\n", name) < 0) { return false; }
        for (const ScaffoldMember *member = type->members; member != NULL; member = member->next) {
            const char *field_type = member->field->symbol->list ? "AmaHandleList" : "AmaHandle";
            if (fprintf(output, "    %s %s;\n", field_type, member->identity->internal_name) < 0) { return false; }
        }
        if (type->plan->type->module->syntax.sections.kind == MODULE_NATIVE) {
            if (fprintf(output, "    %s_State payload;\n", type->native_prefix->internal_name) < 0) { return false; }
        } else if (fputs("    unsigned char payload;\n", output) == EOF) { return false; }
        if (fprintf(output, "} %s_Storage;\n", name) < 0) { return false; }
    }
    return true;
}
