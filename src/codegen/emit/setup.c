/**
 * @file
 * @brief Emit immutable aligned context type schemas and explicitly present lifecycle hooks.
 *
 * [LIFE-01](../../../spec/implementation/native-interface.md#life-01)
 * Derive native offset and alignment from real generated C storage. The existing context factory owns and validates mutable pool descriptors.
 */
#include "codegen/internal.h"

/**
 * Emit only declared initialize/destroy/literal/clone function pointers, or an empty hook set
 * for Ama types. Condition/index remain separate protocols.
 */
static bool hooks(FILE *output, const ScaffoldType *type) {
    if (type->plan->type->module->syntax.sections.kind != MODULE_NATIVE) { return fputs("{0}", output) != EOF; }
    const char *name = type->names[SCAFFOLD_WRITE]->internal_name;
    const char *const fields[] = {"initialize", "destroy", "literal", "clone"};
    const char *const roles[] = {"Default", "Destroy", "Literal", "Clone"};
    if (fputs("{", output) == EOF) { return false; }
    for (size_t operation = 0; operation < 4; operation++) {
        if (type->plan->type->abi.bindings[operation].present
            && fprintf(output, ".%s = %s_%s,", fields[operation], name, roles[operation]) < 0) { return false; }
    }
    return fputc('}', output) != EOF;
}

/**
 * Emit immutable context schemas with actual C layout, explicit slot/generation limits
 * and typed hooks. The existing context factory validates and owns each mutable pool.
 */
bool codegen_emit_type_setup(NativeScaffold *scaffold, FILE *output) {
    if (fputs("static const AmaContextType ama_project_types[] = {\n", output) == EOF) { return false; }
    for (const ScaffoldType *type = scaffold->types; type != NULL; type = type->next) {
        if (!type->plan->requires_pool) { continue; }
        const char *name = type->names[SCAFFOLD_WRITE]->internal_name;
        if (fprintf(output, "{.type_id=UINT64_C(%zu), .size=sizeof(%s_Storage), .alignment=_Alignof(%s_Storage),\n"
                ".slots_per_block=8, .generation_limit=UINT64_MAX, .native_offset=offsetof(%s_Storage,payload),\n"
                ".operation=UINT64_C(%zu), .field_count=%zu, .hooks=",
                codegen_type_id(scaffold, type), name, name, name,
                type->plan->complete->selector, type->plan->field_count) < 0
            || !hooks(output, type)) { return false; }
        if (type->members != NULL && fprintf(output, ", .fields=%s_Fields, .declarations=%s_ProgramFields", name, name) < 0) { return false; }
        if (type->plan->type->abi.bindings[ABI_CONDITION].present
            && fprintf(output, ", .condition=%s_Condition", name) < 0) { return false; }
        if (type->plan->type->abi.bindings[ABI_INDEX].present
            && fprintf(output, ", .index=%s_Index", name) < 0) { return false; }
        if (fputs("},\n", output) == EOF) { return false; }
    }
    return fputs("};\n", output) != EOF;
}
