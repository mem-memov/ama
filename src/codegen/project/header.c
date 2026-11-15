/**
 * @file
 * @brief Emit the retained assembly header and generated host declarations.
 *
 * [STACK-04](../../../spec/implementation/compiler.md#stack-04)
 * Runtime implementations are included once in the final project translation unit. This internal host API is not the future checked component ABI.
 * [PAR-01](../../../spec/language/async-and-parallelism.md#par-01) keeps mutable
 * pool/constant/execution ownership inside independent runtime contexts.
 */
#include "codegen/internal.h"

/**
 * Write runtime/typed storage/native adapters, descriptors, controls and locations into the
 * retained single-TU header. Host declarations are internal; immutable schemas feed independent runtime contexts.
 */
bool codegen_emit_header(NativeScaffold *scaffold, FILE *output, const ScaffoldType *unused) {
    (void)unused;
    if (fputs("#ifndef AMA_PROJECT_ASSEMBLY_H\n#define AMA_PROJECT_ASSEMBLY_H\n"
            "#include <stdio.h>\n#include <stdlib.h>\n#include \"ama-native-types.h\"\n"
            "#include \"runtime/assembly.h\"\n#include \"runtime/program/model.h\"\n#include \"runtime/program/program.h\"\n#include \"runtime/context/execution/execution.h\"\n", output) == EOF
        || !codegen_emit_storage(scaffold, output)) { return false; }
    for (const ScaffoldType *type = scaffold->types; type != NULL; type = type->next) {
        if (type->plan->type->module->syntax.sections.kind == MODULE_NATIVE
            && !codegen_emit_native(scaffold, output, type)) { return false; }
    }
    return codegen_emit_descriptors(scaffold, output) && codegen_emit_program(scaffold, output) && codegen_emit_type_setup(scaffold, output) && codegen_emit_views(scaffold, output) && codegen_emit_controls(scaffold, output) && codegen_emit_locations(scaffold, output) && codegen_emit_debugger(scaffold, output) && fputs(
        "#include \"runtime/metadata/host.h\"\n"
        "typedef struct { AmaContextRegistry registry; uint64_t identity; AmaHandle root; AmaStatus status;\n"
        "    AmaContextView active; _Atomic bool executing; bool initialized, disposed; } AmaProjectHost;\n"
        "AmaHostResult ama_project_execute(AmaProjectHost *, AmaAccessMode);\n"
        "AmaStatus ama_project_dispose(AmaProjectHost *);\n", output) != EOF
        && fprintf(output, "static const AmaContextDefinition ama_project_definition = {\n"
            ".types=ama_project_types,.type_count=sizeof(ama_project_types)/sizeof(ama_project_types[0]),\n"
            ".constants=ama_project_constants,.constant_count=%zu,.initialization_operation=UINT64_C(%zu),\n"
            ".locations=ama_project_locations,.location_count=sizeof(ama_project_locations)/sizeof(ama_project_locations[0])};\n#endif\n",
            scaffold->frontend->semantic.constant_count, scaffold->selected->plan->complete->selector) >= 0;
}
