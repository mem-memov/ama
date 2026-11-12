/**
 * @file
 * @brief Emit standalone entry policy and diagnose an unhandled root yield.
 *
 * [RT-16](../../../../spec/implementation/runtime.md#rt-16)
 * The launcher always disposes initialized resources and derives exit status from complete execution/cleanup status.
 */
#include "codegen/internal.h"

/**
 * Write standalone main that enters once, diagnoses an unhandled root yield, always disposes and
 * returns execution/cleanup status.
 */
bool codegen_emit_launcher(NativeScaffold *scaffold, FILE *output, const ScaffoldType *unused) {
    (void)scaffold;
    (void)unused;
    return fputs("#include \"project.h\"\n#include \"project.c\"\n\nint main(void) {\n"
        "    AmaProjectHost host={0};\n"
        "    AmaHostResult result=ama_project_execute(&host,AMA_ACCESS_ENTRY);\n"
        "    if(result.outcome==AMA_HOST_YIELDED) {\n"
        "        host.status=ama_status_make(AMA_INVALID_STATE,\"Unhandled root yield\",20);\n"
        "        ama_program_report(&host.program,host.status);\n"
        "    }\n"
        "    AmaStatus status=ama_project_dispose(&host);\n"
        "    return status.code==AMA_OK ? EXIT_SUCCESS : EXIT_FAILURE;\n}\n", output) != EOF;
}
