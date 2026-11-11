#include "codegen/internal.h"

bool codegen_emit_header(NativeScaffold *scaffold, FILE *output, const ScaffoldType *unused) {
    (void)unused;
    if (fputs("#ifndef AMA_PROJECT_ASSEMBLY_H\n#define AMA_PROJECT_ASSEMBLY_H\n"
            "#include <stdio.h>\n#include <stdlib.h>\n#include \"ama-native-types.h\"\n"
            "#include \"runtime/assembly.h\"\n#include \"runtime/program/model.h\"\n#include \"runtime/program/constants.h\"\n", output) == EOF
        || !codegen_emit_storage(scaffold, output)) { return false; }
    for (const SharedStorage *shared = scaffold->frontend->plan.shared_first; shared != NULL; shared = shared->next) {
        const char *name = codegen_constant_name(scaffold, shared->constant);
        if (name == NULL || fprintf(output, "static AmaConstantValue %s;\n", name) < 0) { return false; }
    }
    for (const ScaffoldType *type = scaffold->types; type != NULL; type = type->next) {
        if (type->plan->type->module->syntax.sections.kind == MODULE_NATIVE
            && !codegen_emit_native(scaffold, output, type)) { return false; }
    }
    return codegen_emit_descriptors(scaffold, output) && codegen_emit_type_setup(scaffold, output) && fputs("#endif\n", output) != EOF;
}
