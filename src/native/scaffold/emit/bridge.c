/** @file
 * @brief Bind the exact standard-root integer's fixed header-declared size constructor with an independent generated C function type.
 * [PROTO-03](../../../../spec/implementation/native-interface.md#proto-03)
 * makes from_size an additional typed standard contract, not an ABI-table slot.
 * [NAME-05](../../../../spec/implementation/native-interface.md#name-05)
 * keeps the authored helper module-qualified; generated signature names use the
 * distinct type namespace and reserve no extra authored local identifier.
 */
#include "native/scaffold/internal.h"

/** Emit a typed use, never a replacement declaration that could hide a missing authored header prototype. C rejects absent or incompatible names before a caller can use the helper. */
bool scaffold_emit_bridge(FILE *output, const ScaffoldType *type) {
    if (!scaffold_size_bridge(type)) { return true; }
    const char *name = type->names[SCAFFOLD_WRITE]->internal_name;
    const char *prefix = type->native_prefix->internal_name;
    return fprintf(output,
        "typedef AmaStatus %s_FromSizeSignature(%s_State *destination, size_t value);\n"
        "_Static_assert(_Generic(&%s_from_size, %s_FromSizeSignature *: 1, default: 0),\n"
        "    \"The ..integer header must declare AmaStatus from_size(State *, size_t)\");\n"
        "static %s_FromSizeSignature *const %s_FromSize __attribute__((unused)) = &%s_from_size;\n",
        name, prefix, prefix, name, name, name, prefix) >= 0;
}

/** Add the fixed prototype to a newly requested standard integer representation header; existing authored headers remain untouched. */
bool scaffold_emit_bridge_declaration(FILE *output, const ScaffoldType *type) {
    return !scaffold_size_bridge(type)
        || fputs("AmaStatus AMA_NATIVE(from_size)(AMA_NATIVE(State) *destination, size_t value);\n", output) != EOF;
}

/** Emit an honest unwritten bridge only in a requested body template, without extending the six-record ABI or constructing a fake value. */
bool scaffold_emit_bridge_body(FILE *output, const ScaffoldType *type) {
    return !scaffold_size_bridge(type) || fputs(
        "AmaStatus AMA_NATIVE(from_size)(AMA_NATIVE(State) *destination, size_t value) {\n"
        "    (void)destination; (void)value;\n"
        "    /* TODO: construct the exact nonnegative size value in fresh native storage. */\n"
        "    return ama_status_make(AMA_NATIVE_FAILURE, \"Native implementation is not written\",\n"
        "        sizeof(\"Native implementation is not written\") - 1);\n}\n\n", output) != EOF;
}
