/**
 * @file
 * @brief Include typed native bodies and emit lifecycle/conversion adapters.
 *
 * [CABI-03](../../../spec/implementation/native-interface.md#cabi-03)
 * An exact entry signature declaration precedes the body. Read-only protocols receive const payloads and remain distinct from body dispatch.
 * [PROTO-03](../../../spec/implementation/native-interface.md#proto-03) binds
 * the exact standard integer's header-declared size bridge through a typed cast.
 */
#include "codegen/internal.h"

/**
 * Emit one present native protocol adapter with exact constness and extra argument type. Absent
 * protocol emits nothing; native entry dispatch is separate.
 */
static bool hook(FILE *output, const ScaffoldType *type, AbiOperation operation) {
    const AbiBinding *binding = &type->plan->type->abi.bindings[operation];
    if (!binding->present) { return true; }
    const char *name = type->names[SCAFFOLD_WRITE]->internal_name;
    const char *const roles[] = {"Default", "Destroy", "Literal", "Clone", "Condition", "Index"};
    const char *extra = operation == ABI_LITERAL ? ", AmaByteView literal" : operation == ABI_CLONE
        ? ", const void *source" : operation == ABI_CONDITION ? ", bool *result" : operation == ABI_INDEX ? ", size_t *result" : "";
    bool read = operation == ABI_CONDITION || operation == ABI_INDEX;
    if (fprintf(output, "static AmaStatus %s_%s(%svoid *payload%s) {\n    return ",
            name, roles[operation], read ? "const " : "", extra) < 0
        || !native_namespace_symbol(output, type->native_prefix, binding->function)
        || fprintf(output, "((%s%s_State *)payload", read ? "const " : "", type->native_prefix->internal_name) < 0) { return false; }
    if (operation == ABI_LITERAL && fputs(", literal", output) == EOF) { return false; }
    if (operation == ABI_CLONE && fprintf(output, ", (const %s_State *)source", type->native_prefix->internal_name) < 0) { return false; }
    if (read && fputs(", result", output) == EOF) { return false; }
    return fputs(");\n}\n", output) != EOF;
}

/**
 * Include the independent interface, require exact entry signature compatibility before the
 * authored body, then emit present lifecycle/conversion adapters.
 */
bool codegen_emit_native(NativeScaffold *scaffold, FILE *output, const ScaffoldType *type) {
    (void)scaffold;
    if (!scaffold_include(output, type->interface_name)
        || fprintf(output,"static %s_EntrySignature %s_entry;\n",type->native_prefix->internal_name,type->native_prefix->internal_name)<0
        || !scaffold_include(output, type->body_path)) { return false; }
    for (AbiOperation operation = ABI_DEFAULT; operation < ABI_BINDING_COUNT; operation++) {
        if (!hook(output, type, operation)) { return false; }
    }
    if (scaffold_size_bridge(type)
        && fprintf(output, "static inline AmaStatus %s_FromSizeValue(void *payload,size_t value) {\n"
            "return %s_from_size((%s_State *)payload,value);\n}\n",
            type->names[SCAFFOLD_WRITE]->internal_name, type->native_prefix->internal_name,
            type->native_prefix->internal_name) < 0) { return false; }
    return true;
}
