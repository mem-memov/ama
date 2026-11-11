#include "native/scaffold/internal.h"

bool scaffold_emit_interface(NativeScaffold *scaffold, FILE *output, const ScaffoldType *type) {
    (void)scaffold;
    const char *prefix = type->native_prefix->internal_name;
    if (fprintf(output, "#ifndef %s_INTERFACE_H\n#define %s_INTERFACE_H\n", prefix, prefix) < 0
        || !scaffold_include(output, "ama-native-types.h") || !scaffold_include(output, type->begin_name)) {
        return false;
    }
    if (fprintf(output, "typedef void AMA_NATIVE(EntrySignature)(AMA_NATIVE(State) *state,\n"
        "    const %s *parameters, %s *results,\n"
        "    const AmaNativeServices *services, AmaStatus *status);\n",
        type->names[SCAFFOLD_PARAMETERS]->internal_name, type->names[SCAFFOLD_RESULTS]->internal_name) < 0) {
        return false;
    }
    for (AbiOperation operation = ABI_DEFAULT; operation < ABI_BINDING_COUNT; operation++) {
        const AbiBinding *binding = &type->plan->type->abi.bindings[operation];
        if (binding->present && scaffold_hook_first(type, operation) && (!scaffold_emit_signature(output, type, operation, binding->function)
                || fputs(";\n", output) == EOF)) {
            return false;
        }
    }
    return scaffold_include(output, type->end_name) && fputs("#endif\n", output) != EOF;
}
