/** @file
 * @brief Keep one executor owner from root admission through label dispatch and finalization.
 * [EMBED-03](../../../spec/implementation/c-library.md#embed-03) forbids an
 * admission gap during output construction; [STACK-01](../../../spec/implementation/compiler.md#stack-01)
 * executes every Ama call as a label transition inside the shared dispatcher.
 */
#include "internal.h"
void library_execution_runner(LibraryCodecs *codecs) {
    LibraryHeader *writer=&codecs->output;
    const char *prefix=writer->prefix;
    library_header_print(writer,
        "#include <assert.h>\n"
        "static inline bool %s_overlap(const void *left,size_t left_size,const void *right,size_t right_size) {\n"
        "    if (left==NULL || right==NULL) { return false; }\n"
        "    uintptr_t a=(uintptr_t)left,b=(uintptr_t)right;\n"
        "    return a>=b ? a-b<right_size : b-a<left_size;\n}\n"
        "static inline AmaLibraryResult %s_run(AmaLibraryAccess *access,AmaHandle target,const AmaProgramInvocation *invocation,SourceArena *inputs,AmaHostOutcome *outcome) {\n"
        "    AmaContextExecution execution={0};\n"
        "    AmaStatus moved=ama_context_lease_move(&execution.lease,&access->lease);\n"
        "    assert(moved.code==AMA_OK); (void)moved;\n"
        "    AmaStatus admitted=ama_context_execution_begin_leased(&execution,(AmaContextReference){execution.lease.identity,target},invocation);\n"
        "    if (inputs!=NULL) { source_arena_destroy(inputs); }\n"
        "    if (admitted.code!=AMA_OK) {\n"
        "        moved=ama_context_lease_move(&access->lease,&execution.lease); assert(moved.code==AMA_OK);\n"
        "        return ama_library_result(admitted.code==AMA_BUSY ? AMA_LIBRARY_BUSY : AMA_LIBRARY_REFUSED,admitted);\n    }\n"
        "    AmaHostResult dispatched={.outcome=AMA_HOST_FAILED,.status=execution.status};\n"
        "    if (execution.status.code==AMA_OK) { dispatched=ama_project_dispatch(access->view,execution.selector); }\n"
        "    AmaContextOutcome completed={0};\n"
        "    AmaStatus finished=ama_context_execution_finish_retained(&execution,dispatched.outcome,dispatched.status,&completed);\n"
        "    assert(finished.code==AMA_OK); (void)finished;\n"
        "    moved=ama_context_lease_move(&access->lease,&execution.lease); assert(moved.code==AMA_OK);\n"
        "    if (completed.outcome==AMA_HOST_FAILED) { ama_library_forget_roots(&%s_service,access->record); }\n"
        "    *outcome=completed.outcome; return ama_library_result(AMA_LIBRARY_OK,completed.status);\n}\n",
        prefix,prefix,prefix);
}
