#include "codegen/internal.h"

static bool hooks(FILE *output, const ScaffoldType *type) {
    if (type->plan->type->module->syntax.sections.kind != MODULE_NATIVE) { return fputs("(AmaPayloadHooks){0}", output) != EOF; }
    const char *name = type->names[SCAFFOLD_WRITE]->internal_name;
    const char *const fields[] = {"initialize", "destroy", "literal", "clone"};
    const char *const roles[] = {"Default", "Destroy", "Literal", "Clone"};
    if (fputs("(AmaPayloadHooks){", output) == EOF) { return false; }
    for (size_t operation = 0; operation < 4; operation++) {
        if (type->plan->type->abi.bindings[operation].present
            && fprintf(output, ".%s = %s_%s,", fields[operation], name, roles[operation]) < 0) { return false; }
    }
    return fputc('}', output) != EOF;
}

bool codegen_emit_type_setup(NativeScaffold *scaffold, FILE *output) {
    if (fputs("static AmaStatus ama_project_prepare_types(AmaWorld *world) {\n"
            "    AmaStatus status = ama_status_success();\n    (void)world;\n", output) == EOF) { return false; }
    for (const ScaffoldType *type = scaffold->types; type != NULL; type = type->next) {
        if (!type->plan->requires_pool) { continue; }
        const char *name = type->names[SCAFFOLD_WRITE]->internal_name;
        if (fprintf(output, "    _Static_assert(offsetof(%s_Storage, payload) <= sizeof(%s_Storage), \"Invalid native payload layout\");\n"
                "    status = ama_object_type_init_aligned(&%s_Type, world->allocator, UINT64_C(%zu),\n"
                "        sizeof(%s_Storage), _Alignof(%s_Storage), 8, UINT64_MAX, ",
                name, name, name, codegen_type_id(scaffold, type), name, name) < 0) { return false; }
        if (type->members == NULL) {
            if (fputs("NULL", output) == EOF) { return false; }
        } else if (fprintf(output, "%s_Fields", name) < 0) { return false; }
        if (fprintf(output, ", %zu, offsetof(%s_Storage, payload), ", type->plan->field_count, name) < 0
            || !hooks(output, type) || fputs(");\n    if (status.code != AMA_OK) { return status; }\n", output) == EOF) { return false; }
        /* Program descriptors retain both optional conversion hooks through typed uses. */
        for (AbiOperation operation = ABI_CONDITION; operation <= ABI_INDEX; operation++) {
            if (type->plan->type->abi.bindings[operation].present && fprintf(output, "    (void)%s_%s;\n", name,
                    operation == ABI_CONDITION ? "Condition" : "Index") < 0) { return false; }
        }
    }
    return fputs("    return status;\n}\n", output) != EOF;
}
