/**
 * @file
 * @brief Emit fail-first native skeletons and complete explicit ABI binding records.
 *
 * [CABI-06](../../../../spec/implementation/native-interface.md#cabi-06)
 * [PROTO-03](../../../../spec/implementation/native-interface.md#proto-03) adds
 * the standard integer prototype/stub while the ABI record universe remains six.
 * A skeleton includes its own interface; compatible C definition deduplication must not remove mandatory + or - metadata.
 */
#include "native/scaffold/internal.h"

/** Emit one compatible protocol function definition with exact typed signature and an explicit unwritten-native failure result.
 */
static bool hook(FILE *output, AbiOperation operation, const AbiBinding *binding) {
    if (!scaffold_emit_signature(output, operation, binding->function) || fputs(" {\n", output) == EOF) {
        return false;
    }
    const char *unused = operation == ABI_DEFAULT ? "    (void)destination;\n"
        : operation == ABI_DESTROY ? "    (void)state;\n"
        : operation == ABI_LITERAL ? "    (void)destination;\n    (void)literal;\n"
        : operation == ABI_CLONE ? "    (void)destination;\n    (void)source;\n"
        : "    (void)state;\n    (void)result;\n";
    return fputs(unused, output) != EOF
        && fputs("    /* TODO: implement this native contract and its failure cleanup. */\n"
            "    return ama_status_make(AMA_NATIVE_FAILURE, \"Native implementation is not written\",\n"
            "        sizeof(\"Native implementation is not written\") - 1);\n}\n\n", output) != EOF;
}

bool scaffold_emit_body(NativeScaffold *scaffold, FILE *output, const ScaffoldType *type) {
    (void)scaffold;
    if (!scaffold_include(output, type->interface_name) || !scaffold_include(output, type->begin_name)
        || fprintf(output, "\nAMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state,\n"
            "    const %s *parameters, %s *results,\n"
            "    const AmaNativeServices *services, AmaStatus *status) {\n"
            "    (void)state;\n    (void)parameters;\n    (void)results;\n    (void)services;\n"
            "    /* TODO: read typed inputs and update live results under their contracts. */\n"
            "    *status = ama_status_make(AMA_NATIVE_FAILURE, \"Native implementation is not written\",\n"
            "        sizeof(\"Native implementation is not written\") - 1);\n}\n\n",
            type->names[SCAFFOLD_PARAMETERS]->internal_name, type->names[SCAFFOLD_RESULTS]->internal_name) < 0) {
        return false;
    }
    for (AbiOperation operation = ABI_DEFAULT; operation < ABI_BINDING_COUNT; operation++) {
        const AbiBinding *binding = &type->plan->type->abi.bindings[operation];
        if (binding->present && scaffold_hook_first(type, operation) && !hook(output, operation, binding)) {
            return false;
        }
    }
    return scaffold_emit_bridge_body(output, type) && scaffold_include(output, type->end_name);
}

bool scaffold_emit_payload(NativeScaffold *scaffold, FILE *output, const ScaffoldType *type) {
    (void)scaffold;
    const char *prefix = type->native_prefix->internal_name;
    return fprintf(output, "#ifndef %s_PAYLOAD_H\n#define %s_PAYLOAD_H\n"
        "/* Included in this module's generated AMA_NATIVE context. */\n"
        "struct AMA_NATIVE(State) {\n"
        "    /* TODO: declare native-owned data; persistent Ama links belong in Ama fields. */\n"
        "    unsigned char reserved;\n};\n", prefix, prefix) >= 0
        && scaffold_emit_bridge_declaration(output, type) && fputs("#endif\n", output) != EOF;
}

bool scaffold_emit_abi(NativeScaffold *scaffold, FILE *output, const ScaffoldType *type) {
    (void)scaffold;
    const char *const symbols[] = {"+", "-", "|", "=", "?", "[]"};
    for (AbiOperation operation = ABI_DEFAULT; operation < ABI_BINDING_COUNT; operation++) {
        const AbiBinding *binding = &type->plan->type->abi.bindings[operation];
        if (binding->present && (fprintf(output, "%s ", symbols[operation]) < 0
                || fwrite(source_bytes(binding->function.source) + binding->function.start,
                    1, binding->function.end - binding->function.start, output) != binding->function.end - binding->function.start
                || fputc('\n', output) == EOF)) {
            return false;
        }
    }
    return true;
}
