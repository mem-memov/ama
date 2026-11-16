/** @file
 * @brief Dispatch one explicit read-only preview, durable apply or independent journal recovery and report outcome before releasing owners.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * separates commit from acknowledgement; output failure never authorizes rollback of a committed move.
 */
#include <stdlib.h>
#include "internal.h"
#include "diagnostics/render/render.h"

/** Completed source mutation and output acknowledgement are independent; retain the receipt and explain the known outcome. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
static int acknowledge(ToolJsonWriter *writer, bool operation, bool rendered, bool known, bool committed,
    const char *journal, const Diagnostic *primary, const Diagnostic *cleanup, FILE *errors) {
    bool flushed = tool_json_finish(writer);
    if (primary != NULL && primary->message != NULL) { (void)diagnostic_render(errors,primary); }
    if (cleanup != NULL && cleanup->message != NULL) { (void)diagnostic_render(errors,cleanup); }
    if (!rendered || !flushed) {
        Diagnostic output = {.code=DIAGNOSTIC_IO_FAILURE,.primary={.path=journal},
            .message=known && committed ? "The relocation committed; its result output failed. The recovery receipt is retained"
                : !known ? "The relocation outcome is unresolved; result output failed. Retained journal data requires recovery"
                : "The relocation result output could not be acknowledged"};
        (void)diagnostic_render(errors,&output);
    }
    return operation && rendered && flushed ? EXIT_SUCCESS : EXIT_FAILURE;
}
/** A recovered receipt's complete authority determines the outcome; a failure before validation leaves commit explicitly unknown. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
static int recover(const DriverOptions *options, ToolJsonWriter *writer, FILE *errors) {
    RelocationRecovery recovery;
    relocation_recovery_init(&recovery,options->arena.allocator);
    bool complete = relocation_recovery_run(&recovery,&(RelocationRecoveryRequest){.journal=options->journal});
    const RelocationTransaction *transaction = &recovery.transaction;
    const char *journal = transaction->journal == NULL ? options->journal : transaction->journal;
    const char *state = !complete ? "incomplete" : transaction->committed ? "committed" : "rolled-back";
    bool rendered = driver_relocation_result(writer,"recover",complete,transaction->status,state,recovery.ready,transaction->committed,recovery.finished,
        &transaction->report,NULL,journal,&transaction->error,&transaction->cleanup_error);
    int status = acknowledge(writer,complete,rendered,recovery.ready,transaction->committed,journal,&transaction->error,&transaction->cleanup_error,errors);
    relocation_recovery_destroy(&recovery);
    return status;
}
/** Readiness alone permits no filesystem write; apply explicitly supplies the recovery directory and durable transaction owner. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
static int prepared(const DriverOptions *options, ToolJsonWriter *writer, FILE *errors) {
    DriverRelocationPlan plan;
    driver_relocation_plan_init(&plan,options->arena.allocator);
    bool ready = driver_relocation_plan_build(&plan,options);
    RelocationTransaction transaction;
    relocation_transaction_init(&transaction,options->arena.allocator);
    bool applying = options->request == DRIVER_REQUEST_RELOCATE_APPLY;
    bool complete = ready;
    if (ready && applying) {
        complete = relocation_transaction_apply(&transaction,&(RelocationTransactionRequest){.staging=&plan.staging,.recovery_directory=options->recovery_directory});
    }
    const Diagnostic *primary = ready && applying ? &transaction.error : plan.error;
    const Diagnostic *cleanup = ready && applying ? &transaction.cleanup_error : NULL;
    bool known = !transaction.decision_pending;
    bool finished = !applying || !ready || (complete || transaction.state == RELOCATION_TRANSACTION_ROLLED_BACK);
    const char *state = !ready ? "rejected" : !applying ? "ready"
        : transaction.committed ? (complete ? "committed" : "incomplete")
        : transaction.state == RELOCATION_TRANSACTION_ROLLED_BACK ? "rolled-back" : "incomplete";
    bool rendered = driver_relocation_result(writer,applying ? "apply" : "preview",complete,ready && applying ? transaction.status : plan.status,state,known,transaction.committed,finished,
        &plan.report,&plan.references,transaction.journal,primary,cleanup);
    int status = acknowledge(writer,complete,rendered,known,transaction.committed,transaction.journal,primary,cleanup,errors);
    relocation_transaction_destroy(&transaction);
    driver_relocation_plan_destroy(&plan);
    return status;
}
/** Every owner remains alive through response and diagnostic rendering; destruction performs no hidden filesystem action. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
int driver_relocation(const DriverOptions *options, FILE *output, FILE *errors) {
    ToolJsonWriter writer;
    tool_json_init(&writer,output);
    return options->request == DRIVER_REQUEST_RELOCATE_RECOVER ? recover(options,&writer,errors) : prepared(options,&writer,errors);
}
