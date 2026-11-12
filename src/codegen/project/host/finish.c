/**
 * @file
 * @brief Emit explicit host outcomes and terminal error reporting/disposal.
 *
 * [RT-14](../../../../spec/implementation/runtime.md#rt-14)
 * Report the active chain before disposal removes it. Root yield remains a resumable host outcome rather than implicit scheduling.
 */
#include "codegen/internal.h"

/**
 * Write common completion/yield/error outcomes. Report the active chain before terminal
 * disposal, clear execution admission and expose an empty root on failure.
 */
bool codegen_emit_host_finish(FILE *output) {
    return fputs("ama_project_finish:;\n"
        "host->executing=false; host->status=ama_status;\n"
        "if(ama_status.code!=AMA_OK) {\n"
        "    ama_outcome=AMA_HOST_FAILED;\n"
        "    if(!ama_program.failure_reported) ama_program_report(&ama_program,ama_status);\n"
        "    (void)ama_project_dispose(host);\n"
        "}\n"
        "return (AmaHostResult){.outcome=ama_outcome,.root=ama_outcome==AMA_HOST_FAILED ? (AmaHandle){0} : host->root,.status=host->status};\n"
        "#undef ama_program\n#undef ama_world\n}\n", output) != EOF;
}
