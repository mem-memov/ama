/** @file
 * @brief Compose exclusive revalidation, complete preparation, ordered publication and inverse cleanup.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * keeps originals until commit and reports incomplete recovery instead of claiming a partly applied move succeeded.
 */
#include "internal.h"

/** Precommit failures restore every attempted change; an uncertain commit decision retains all copies for explicit recovery. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
static bool abort_transaction(RelocationTransaction *transaction) {
    transaction->cleaning = true;
    bool restored = !transaction->decision_pending && relocation_transaction_rollback(transaction)
        && relocation_transaction_restored(transaction);
    bool cleaned = restored && relocation_transaction_cleanup(transaction);
    if (cleaned) {
        cleaned = transaction->retain_receipt ? relocation_transaction_finish(transaction) : relocation_transaction_abandoned(transaction);
    }
    if (cleaned) { transaction->state = RELOCATION_TRANSACTION_ROLLED_BACK; }
    else {
        transaction->state = RELOCATION_TRANSACTION_INCOMPLETE;
        transaction->status = RELOCATION_INCOMPLETE_TRANSACTION;
    }
    transaction->cleaning = false;
    relocation_transaction_release(transaction);
    return false;
}
/** The final rebuild runs after temporary preparation too, so no selected input can change unnoticed during a long copy phase. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_transaction_apply(RelocationTransaction *transaction, const RelocationTransactionRequest *request) {
    if (transaction->attempted) { return false; }
    transaction->attempted = true;
    if (request == NULL || request->staging == NULL || !request->staging->ready) {
        return relocation_transaction_fail(transaction,RELOCATION_INVALID_PATH,"A complete validated relocation staging plan is required",NULL);
    }
    transaction->staging = request->staging;
    transaction->io = request->io;
    transaction->state = RELOCATION_TRANSACTION_PREPARING;
    if (!relocation_report_build(&transaction->report,request->staging)) {
        relocation_transaction_fail(transaction,transaction->report.status,"Relocation report ownership could not be prepared",NULL);
        return abort_transaction(transaction);
    }
    if (!relocation_transaction_lease(transaction,request->recovery_directory)
        || !relocation_transaction_revalidate(transaction) || !relocation_transaction_prepare(transaction)
        || !relocation_transaction_revalidate(transaction) || !relocation_transaction_journal(transaction)) {
        return abort_transaction(transaction);
    }
    transaction->state = RELOCATION_TRANSACTION_PREPARED;
    transaction->state = RELOCATION_TRANSACTION_APPLYING;
    if (!relocation_transaction_publish(transaction) || !relocation_transaction_decide(transaction)) {
        return abort_transaction(transaction);
    }
    transaction->cleaning = true;
    bool complete = relocation_transaction_cleanup(transaction) && relocation_transaction_finish(transaction);
    transaction->cleaning = false;
    if (!complete) {
        transaction->state = RELOCATION_TRANSACTION_INCOMPLETE;
        transaction->status = RELOCATION_INCOMPLETE_TRANSACTION;
    }
    relocation_transaction_release(transaction);
    return complete;
}
