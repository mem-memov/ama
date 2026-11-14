/** @file
 * @brief Emit explicit first-root startup and existing-root entry/resume under one context lease.
 * [RT-11](../../../../spec/implementation/runtime.md#rt-11) constructs constants
 * before the explicit root. [CALL-08](../../../../spec/language/calls-and-results.md#call-08)
 * resets interfaces only for a new invocation of an existing root, never twice
 * for its first construction. Admission rejection leaves existing Ama state intact.
 */
#include "codegen/internal.h"

/** Create one independent context from immutable schemas, then hold the existing runtime execution lease through every generated label. */
bool codegen_emit_host_initialize(NativeScaffold *scaffold, FILE *output) {
    return fprintf(output,
        "if(!host->initialized) {\n"
        "ama_status=ama_context_registry_init(&host->registry,source_system_allocator());\n"
        "if(ama_status.code==AMA_OK) { host->initialized=true; ama_status=ama_context_create(&host->registry,&ama_project_definition,&host->identity); }\n"
        "if(ama_status.code==AMA_OK) ama_status=ama_context_execution_start(&host->registry,host->identity,UINT64_C(%zu),&execution);\n"
        "if(ama_status.code!=AMA_OK) { host->status=ama_status; (void)ama_project_dispose_owned(host);\n"
        "AmaHostResult result={.outcome=AMA_HOST_FAILED,.status=host->status};\n"
        "atomic_store_explicit(&host->executing,false,memory_order_release); return result; }\n"
        "host->root=execution.root.local;\n"
        "} else { AmaProgramInvocation invocation={.mode=mode};\n"
        "ama_status=ama_context_execution_begin(&host->registry,host->identity,(AmaContextReference){host->identity,host->root},&invocation,&execution);\n"
        "if(ama_status.code!=AMA_OK) { AmaHostResult result={.outcome=AMA_HOST_FAILED,.status=ama_status};\n"
        "atomic_store_explicit(&host->executing,false,memory_order_release); return result; } }\n"
        "ama_status=ama_context_inspect(&execution.lease,&view);\n"
        "if(ama_status.code==AMA_OK) ama_status=execution.status;\n"
        "host->active=view; ama_return=execution.selector;\n"
        "#define ama_world (*view.world)\n#define ama_program (*view.program)\n"
        "#define ama_current (view.control->current)\n"
        "if(ama_status.code!=AMA_OK) goto ama_project_finish;\n",
        codegen_type_id(scaffold, scaffold->selected)) >= 0;
}
