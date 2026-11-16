/** @file
 * @brief Replay the persisted outcome once, retaining receipts and all unresolved originals on failure.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * rolls back uncommitted work, retires committed backups and makes terminal recovery a historical no-op.
 */
#include "internal.h"

/** A false result always keeps an incomplete outcome and diagnostics; destruction performs no hidden replay. */
static bool outcome(RelocationRecovery *recovery, bool complete) {
    RelocationTransaction *transaction = &recovery->transaction;
    if (complete) {
        recovery->finished = true;
        recovery->rolled_back = !transaction->committed;
        transaction->state = transaction->committed ? RELOCATION_TRANSACTION_COMMITTED : RELOCATION_TRANSACTION_ROLLED_BACK;
    } else {
        recovery->finished = false;
        transaction->state = RELOCATION_TRANSACTION_INCOMPLETE;
        if (transaction->status == RELOCATION_OK) { transaction->status = RELOCATION_INCOMPLETE_TRANSACTION; }
    }
    relocation_transaction_release(transaction);
    return complete;
}
/** The entire inventory and filesystem preconditions are validated before the first inverse mutation. */
bool relocation_recovery_run(RelocationRecovery *recovery, const RelocationRecoveryRequest *request) {
    if (recovery->attempted) { return false; }
    recovery->attempted = true;
    RelocationTransaction *transaction = &recovery->transaction;
    transaction->attempted = true;
    if (request == NULL || request->journal == NULL) {
        relocation_transaction_fail(transaction,RELOCATION_INVALID_PATH,"Recovery requires an explicit existing journal directory",NULL);
        return outcome(recovery,false);
    }
    transaction->io = request->io;
    if (!relocation_recovery_load(recovery,request->journal)) { return outcome(recovery,false); }
    if (recovery->finished) { return outcome(recovery,relocation_recovery_acknowledge(recovery)); }
    if (recovery->preparing) { return outcome(recovery,relocation_recovery_preparation(recovery)); }
    if (!relocation_recovery_preflight(recovery)) { return outcome(recovery,false); }
    if (!transaction->committed && (!relocation_recovery_retract(recovery)
        || !relocation_transaction_rollback(transaction) || !relocation_transaction_restored(transaction))) { return outcome(recovery,false); }
    if (transaction->committed && !relocation_recovery_acknowledge(recovery)) { return outcome(recovery,false); }
    bool complete = relocation_transaction_cleanup(transaction) && relocation_transaction_finish(transaction);
    return outcome(recovery,complete);
}
