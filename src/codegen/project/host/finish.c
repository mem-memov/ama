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
 * Component contract: [RT-14](../../../../spec/implementation/runtime.md#rt-14). */
bool codegen_emit_host_finish(FILE *output) {
    return fputs("ama_project_finish:;\n"
        "host->active=(AmaContextView){0};\n"
        "AmaContextOutcome completed={0};\n"
        "AmaStatus finished=ama_context_execution_finish(&execution,ama_outcome,ama_status,&completed);\n"
        "host->status=finished.code==AMA_OK ? completed.status : finished;\n"
        "host->root=finished.code==AMA_OK ? completed.root.local : (AmaHandle){0};\n"
        "if(host->status.code!=AMA_OK) (void)ama_project_dispose_owned(host);\n"
        "AmaHostResult result={.outcome=host->status.code==AMA_OK ? completed.outcome : AMA_HOST_FAILED,\n"
        ".root=host->root,.status=host->status};\n"
        "atomic_store_explicit(&host->executing,false,memory_order_release);\n"
        "return result;\n#undef ama_program\n#undef ama_world\n#undef ama_current\n}\n", output) != EOF;
}
