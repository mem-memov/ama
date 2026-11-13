/**
 * @file
 * @brief Emit retained synchronous host admission and disposal ownership checks.
 *
 * [RT-14](../../../../spec/implementation/runtime.md#rt-14)
 * The static single-owner guard reflects current global descriptors; independent context storage and thread-safe admission remain T061.
 */
#include "codegen/internal.h"

/**
 * Write retained owner admission and idempotent disposal with busy checks. Shared static
 * descriptor ownership limits this API to one retained project owner; independent contexts
 * remain T061.
 */
bool codegen_emit_host_context(FILE *output) {
    return fputs(
            "/* One host execution function; Ama bodies are labels below. */\n"
            "static AmaProjectHost *ama_project_owner;\n"
            "AmaStatus ama_project_dispose(AmaProjectHost *host) {\n"
            "    if(host->executing) return ama_status_make(AMA_BUSY,NULL,0);\n"
            "    if(host->disposed) return host->status;\n"
            "    host->executing=true;\n"
            "    if(host->initialized) {\n"
            "        host->status=ama_program_stop(&host->program,host->root,host->status);\n"
            "        if(host->status.code!=AMA_OK && !host->program.failure_reported) ama_program_report(&host->program,host->status);\n"
            "    }\n"
            "    host->disposed=true; host->executing=false; if(ama_project_owner==host) ama_project_owner=NULL;\n"
            "    return host->status;\n}\n"
            "AmaHostResult ama_project_execute(AmaProjectHost *host, AmaAccessMode mode) {\n"
            "    if(host->executing || (ama_project_owner!=NULL && ama_project_owner!=host))\n"
            "        return (AmaHostResult){.outcome=AMA_HOST_FAILED,.status=ama_status_make(AMA_BUSY,NULL,0)};\n"
            "    if(host->disposed || (mode!=AMA_ACCESS_ENTRY && mode!=AMA_ACCESS_RESUME))\n"
            "        return (AmaHostResult){.outcome=AMA_HOST_FAILED,.status=ama_status_make(AMA_INVALID_STATE,NULL,0)};\n"
            "    host->executing=true; ama_project_owner=host;\n"
            "#define ama_world (host->world)\n#define ama_program (host->program)\n"
            "AmaHandle ama_root=host->root, ama_current=host->root;\n"
            "AmaPoolCell *ama_cell=NULL; uint64_t ama_return=0;\n"
            "AmaStatus ama_status=ama_status_success();\n"
            "AmaHostOutcome ama_outcome=AMA_HOST_COMPLETED;\n", output) != EOF;
}
