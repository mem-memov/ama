/**
 * @file
 * @brief Emit an explicit native lifecycle test skeleton using the generated interface.
 *
 * [CABI-06](../../../../spec/implementation/native-interface.md#cabi-06)
 * The template exercises authored initialize/destroy after implementation; it does not certify an unwritten fail-first hook.
 */
#include "native/scaffold/internal.h"

bool scaffold_emit_test(NativeScaffold *scaffold, FILE *output, const ScaffoldType *type) {
    (void)scaffold;
    const AbiBinding *initialize = &type->plan->type->abi.bindings[ABI_DEFAULT];
    const AbiBinding *destroy = &type->plan->type->abi.bindings[ABI_DESTROY];
    if (fputs("#include <assert.h>\n#include <stdlib.h>\n", output) == EOF
        || !scaffold_include(output, type->template_name) || !scaffold_include(output, type->begin_name)
        || fputs("\nstatic void it_initializes_and_destroys_native_state(void) {\n"
            "    AMA_NATIVE(State) state;\n    AmaStatus status = ", output) == EOF
        || !native_namespace_symbol(output, type->native_prefix, initialize->function)
        || fputs("(&state);\n    assert(status.code == AMA_OK);\n    status = ", output) == EOF
        || !native_namespace_symbol(output, type->native_prefix, destroy->function)
        || fputs("(&state);\n    assert(status.code == AMA_OK);\n}\n\n"
            "void run(void) {\n    it_initializes_and_destroys_native_state();\n}\n\n"
            "int main(void) {\n    run();\n    return EXIT_SUCCESS;\n}\n", output) == EOF) {
        return false;
    }
    return scaffold_include(output, type->end_name);
}
