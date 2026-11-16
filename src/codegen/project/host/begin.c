/**
 * @file
 * @brief Emit root entry/resume eligibility and the host return sentinel.
 *
 * [CONT-03](../../../../spec/language/continuations.md#cont-03)
 * Resume reads the saved selector without preparing inputs; execution clears the saved position before dispatching labels.
 */
#include "codegen/internal.h"

/**
 * Compose owner/startup checks, validate root resume eligibility, consume saved position and
 * attach the host return sentinel before label dispatch.
 * Component contract: [CONT-03](../../../../spec/language/continuations.md#cont-03). */
bool codegen_emit_host_begin(NativeScaffold *scaffold, FILE *output) {
    if (!codegen_emit_host_context(output) || !codegen_emit_host_initialize(scaffold, output)) { return false; }
    return fputs(
        "ama_status=ama_program_cell(&ama_program,ama_current,&ama_cell);\n"
        "if(ama_status.code!=AMA_OK) goto ama_project_finish;\n"
        "if(mode==AMA_ACCESS_RESUME) goto ama_project_resume;\n", output) != EOF;
}
