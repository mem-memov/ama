/** @file
 * @brief Emit per-host admission and disposal using isolated checked runtime contexts.
 * [RT-14](../../../../spec/implementation/runtime.md#rt-14) separates rejected
 * admission from accepted execution failure. [PAR-01](../../../../spec/language/async-and-parallelism.md#par-01)
 * gives every host its own pools, constants and execution state; the atomic host
 * gate protects first creation and disposal before a context lease exists.
 */
#include "codegen/internal.h"

/** Emit a cleanup operation requiring the host gate, its public admission wrapper, and execution admission.
 * Registry destruction consumes its initialized owner only after admission. Use
 * that exact postcondition to distinguish refusal from a destructor returning
 * Busy as its technical error; an error code alone cannot encode this provenance.
 */
bool codegen_emit_host_context(FILE *output) {
    return fputs(
        "static AmaStatus ama_project_dispose_owned(AmaProjectHost *host) {\n"
        "if(!host->disposed) {\n"
        "if(host->initialized) { AmaStatus cleanup=ama_context_registry_destroy(&host->registry);\n"
        "if(host->registry.initialized) { return cleanup; }\n"
        "host->status=ama_cleanup_primary(host->status,cleanup); }\n"
        "host->disposed=true; host->root=(AmaHandle){0}; } return host->status; }\n"
        "AmaStatus ama_project_dispose(AmaProjectHost *host) {\n"
        "if(atomic_exchange_explicit(&host->executing,true,memory_order_acquire)) return ama_status_make(AMA_BUSY,NULL,0);\n"
        "AmaStatus status=ama_project_dispose_owned(host);\n"
        "atomic_store_explicit(&host->executing,false,memory_order_release); return status; }\n"
        "AmaHostResult ama_project_execute(AmaProjectHost *host, AmaAccessMode mode) {\n"
        "if(atomic_exchange_explicit(&host->executing,true,memory_order_acquire))\n"
        "return (AmaHostResult){.outcome=AMA_HOST_FAILED,.status=ama_status_make(AMA_BUSY,NULL,0)};\n"
        "if(host->disposed || (mode!=AMA_ACCESS_ENTRY && mode!=AMA_ACCESS_RESUME) || (!host->initialized && mode==AMA_ACCESS_RESUME)) {\n"
        "AmaStatus rejected=ama_status_make(!host->initialized && mode==AMA_ACCESS_RESUME && !host->disposed ? AMA_NO_RESUME : AMA_INVALID_STATE,NULL,0);\n"
        "atomic_store_explicit(&host->executing,false,memory_order_release);\n"
        "return (AmaHostResult){.outcome=AMA_HOST_FAILED,.status=rejected}; }\n"
        "AmaContextExecution execution={0}; AmaContextView view={0};\n"
        "AmaPoolCell *ama_cell=NULL; uint64_t ama_return=0;\n"
        "AmaStatus ama_status=ama_status_success(); AmaHostOutcome ama_outcome=AMA_HOST_COMPLETED;\n", output) != EOF;
}
