/**
 * @file
 * @brief Emit the retained assembly header and generated host declarations.
 *
 * [STACK-04](../../../spec/implementation/compiler.md#stack-04)
 * Runtime implementations are included once in the final project translation unit. This internal host API is not the future checked component ABI.
 */
#include "codegen/internal.h"

/**
 * Write runtime/typed storage/native adapters, descriptors, controls and locations into the
 * retained single-TU header. Host declarations are internal and still use global type/shared
 * descriptors.
 */
bool codegen_emit_header(NativeScaffold *scaffold, FILE *output, const ScaffoldType *unused) {
    (void)unused;
    if (fputs("#ifndef AMA_PROJECT_ASSEMBLY_H\n#define AMA_PROJECT_ASSEMBLY_H\n"
            "#include <stdio.h>\n#include <stdlib.h>\n#include \"ama-native-types.h\"\n"
            "#include \"runtime/assembly.h\"\n#include \"runtime/program/model.h\"\n#include \"runtime/program/program.h\"\n", output) == EOF
        || !codegen_emit_storage(scaffold, output)) { return false; }
    for (const SharedStorage *shared = scaffold->frontend->plan.shared_first; shared != NULL; shared = shared->next) {
        const char *name = codegen_constant_name(scaffold, shared->constant);
        if (name == NULL || fprintf(output, "static AmaConstantStorage %s;\n", name) < 0) { return false; }
    }
    for (const ScaffoldType *type = scaffold->types; type != NULL; type = type->next) {
        if (type->plan->type->module->syntax.sections.kind == MODULE_NATIVE
            && !codegen_emit_native(scaffold, output, type)) { return false; }
    }
    return codegen_emit_descriptors(scaffold, output) && codegen_emit_type_setup(scaffold, output) && codegen_emit_program(scaffold, output) && codegen_emit_views(scaffold, output) && codegen_emit_controls(scaffold, output) && codegen_emit_locations(scaffold, output) && fputs(
        "#include \"runtime/metadata/host.h\"\n"
        "typedef struct { AmaWorld world; AmaProgram program; AmaHandle root; AmaStatus status;\n"
        "    bool initialized, executing, disposed; } AmaProjectHost;\n"
        "AmaHostResult ama_project_execute(AmaProjectHost *, AmaAccessMode);\n"
        "AmaStatus ama_project_dispose(AmaProjectHost *);\n#endif\n", output) != EOF;
}
