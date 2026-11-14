/** @file
 * @brief Verify linear lease transfers and retained execution completion before C result publication.
 */
#include "runtime/context/execution/execution.h"
/** A move preserves the executor gate and rejects self/nonempty/invalid destinations. */
static void it_moves_linear_context_access_without_an_admission_gap(void) {
    setup_contexts();
    uint64_t identity=create_context(), peer=create_context();
    AmaContextLease source={0}, destination={0}, other={0};
    (void)enter_context(identity,&source); (void)enter_context(peer,&other);
    AmaStatus status=ama_context_lease_move(&other,&source);
    assert(status.code==AMA_INVALID_STATE && source.identity==identity && other.identity==peer);
    status=ama_context_lease_move(&source,&source); assert(status.code==AMA_INVALID_STATE);
    status=ama_context_lease_move(NULL,&source); assert(status.code==AMA_INVALID_STATE);
    status=ama_context_lease_move(&destination,&source);
    assert(status.code==AMA_OK && source.context==NULL && destination.identity==identity);
    status=ama_context_dispose(&registry,identity); assert(status.code==AMA_BUSY);
    status=ama_context_lease_move(&source,&destination); assert(status.code==AMA_OK && destination.context==NULL);
    leave_context(&source); leave_context(&other);
    status=ama_context_dispose(&registry,identity); assert(status.code==AMA_OK);
    status=ama_context_dispose(&registry,peer); assert(status.code==AMA_OK);
    finish_contexts();
}
/** Completed/yielded results remain exclusively accessible until the host explicitly releases its lease. */
static void it_finishes_completed_and_yielded_execution_without_releasing_result_access(void) {
    setup_contexts(); uint64_t identity=create_context();
    AmaContextLease lease={0}; AmaContextView view=enter_context(identity,&lease);
    AmaContextReference root=create_root(&lease,view);
    root_value(&lease,root)->native.number=73;
    AmaContextExecution execution={0};
    AmaStatus status=ama_context_lease_move(&execution.lease,&lease); assert(status.code==AMA_OK);
    const AmaProgramInvocation resume={.mode=AMA_ACCESS_RESUME}, entry={.mode=AMA_ACCESS_ENTRY};
    status=ama_context_execution_begin_leased(&execution,root,&resume);
    assert(status.code==AMA_NO_RESUME && execution.lease.identity==identity && execution.root.context==0);
    status=ama_context_execution_begin_leased(&execution,root,&entry);
    assert(status.code==AMA_OK && execution.status.code==AMA_OK && execution.selector==0);
    uint64_t selector=0;
    status=ama_program_complete(view.program,&view.control->current,&selector); assert(status.code==AMA_OK && selector==0);
    AmaContextOutcome result={0};
    status=ama_context_execution_finish_retained(&execution,AMA_HOST_COMPLETED,ama_status_success(),&result);
    assert(status.code==AMA_OK && result.outcome==AMA_HOST_COMPLETED && execution.lease.identity==identity);
    status=ama_context_dispose(&registry,identity); assert(status.code==AMA_BUSY);
    assert(root_value(&execution.lease,root)->native.number==73);
    status=ama_context_execution_begin_leased(&execution,root,&entry); assert(status.code==AMA_OK);
    AmaPoolCell *cell=NULL; status=ama_program_cell(view.program,root.local,&cell); assert(status.code==AMA_OK);
    cell->metadata.service.resume=(AmaResumePosition){true,17};
    status=ama_program_yield(view.program,&view.control->current,&selector); assert(status.code==AMA_OK && selector==0);
    status=ama_context_execution_finish_retained(&execution,AMA_HOST_YIELDED,ama_status_success(),&result);
    assert(status.code==AMA_OK && result.outcome==AMA_HOST_YIELDED && cell->metadata.service.resume.selector==17);
    status=ama_context_execution_begin_leased(&execution,root,&resume);
    assert(status.code==AMA_OK && execution.selector==17);
    status=ama_program_complete(view.program,&view.control->current,&selector); assert(status.code==AMA_OK);
    status=ama_context_execution_finish_retained(&execution,AMA_HOST_COMPLETED,ama_status_success(),&result); assert(status.code==AMA_OK);
    assert(root_value(&execution.lease,root)->native.number==73);
    status=ama_context_lease_move(&lease,&execution.lease); assert(status.code==AMA_OK);
    leave_context(&lease); status=ama_context_dispose(&registry,identity); assert(status.code==AMA_OK);
    finish_contexts();
}
/** Failed completion consumes resources once while keeping the diagnostic record pinned for host bookkeeping. */
static void it_retains_exclusive_diagnostic_access_after_execution_cleanup(void) {
    setup_contexts(); uint64_t identity=create_context();
    AmaContextLease lease={0}; AmaContextView view=enter_context(identity,&lease);
    AmaContextReference root=create_root(&lease,view); AmaContextExecution execution={0};
    AmaStatus status=ama_context_lease_move(&execution.lease,&lease); assert(status.code==AMA_OK);
    const AmaProgramInvocation entry={.mode=AMA_ACCESS_ENTRY};
    status=ama_context_execution_begin_leased(&execution,root,&entry); assert(status.code==AMA_OK);
    AmaContextOutcome outcome={0};
    status=ama_context_execution_finish_retained(&execution,AMA_HOST_FAILED,ama_status_make(AMA_NATIVE_FAILURE,"failure",7),&outcome);
    assert(status.code==AMA_OK && outcome.outcome==AMA_HOST_FAILED && outcome.root.context==0 && destroyed==constructed);
    status=ama_context_inspect(&execution.lease,&view); assert(status.code==AMA_OK && view.status.code==AMA_NATIVE_FAILURE && view.world->types==NULL);
    status=ama_context_dispose(&registry,identity); assert(status.code==AMA_BUSY);
    status=ama_context_lease_move(&lease,&execution.lease); assert(status.code==AMA_OK);
    leave_context(&lease); status=ama_context_dispose(&registry,identity); assert(status.code==AMA_NATIVE_FAILURE);
    finish_contexts();
}
