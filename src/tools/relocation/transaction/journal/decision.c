/** @file
 * @brief Place a commit decision only after published directory entries are synchronized, retracting failed provisional records before rollback.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * distinguishes uncommitted rollback from committed cleanup without rewriting source text again.
 */
#include <fcntl.h>
#include <unistd.h>
#include "../internal.h"

/** A failed commit acknowledgement cannot authorize rollback until its marker is durably absent. Component contract: [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
static bool retract(RelocationTransaction *transaction) {
    bool prior_cleaning = transaction->cleaning;
    transaction->cleaning = true;
    int descriptor = open(transaction->journal_record,O_RDWR|O_NOFOLLOW|O_CLOEXEC);
    bool complete = descriptor >= 0;
    if (!complete) { relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"A provisional commit record could not be reopened",transaction->journal_record); }
    if (complete && (!relocation_transaction_permit(transaction,RELOCATION_IO_JOURNAL,transaction->journal_record)
        || ftruncate(descriptor,transaction->journal_length) != 0)) {
        complete = relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"A provisional commit record could not be retracted",transaction->journal_record);
    }
    if (complete && (!relocation_transaction_permit(transaction,RELOCATION_IO_SYNC,transaction->journal_record) || fsync(descriptor) != 0)) {
        complete = relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"Commit retraction could not be synchronized",transaction->journal_record);
    }
    if (descriptor >= 0 && !relocation_transaction_close(transaction,descriptor,transaction->journal_record)) { complete = false; }
    if (complete && !relocation_transaction_sync(transaction,transaction->journal)) { complete = false; }
    if (complete) { transaction->decision_pending = false; }
    transaction->cleaning = prior_cleaning;
    return complete;
}
/** Parent and slot synchronization precede the record; any uncertain retraction leaves all published data and originals retained. Component contract: [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_transaction_decide(RelocationTransaction *transaction) {
    for (const RelocationTransactionAction *action = transaction->first; action != NULL; action = action->next) {
        if (!relocation_transaction_sync(transaction,action->slot->directory)
            || !relocation_transaction_sync(transaction,action->slot->parent)) { return false; }
    }
    if (!relocation_transaction_permit(transaction,RELOCATION_IO_JOURNAL,transaction->journal_record)) { return false; }
    int descriptor = open(transaction->journal_record,O_WRONLY|O_APPEND|O_NOFOLLOW|O_CLOEXEC);
    if (descriptor < 0) { return relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"The commit record could not be opened",transaction->journal_record); }
    transaction->decision_pending = true;
    bool complete = relocation_transaction_write(transaction,descriptor,transaction->journal_record,(const unsigned char *)"COMMIT\n",7);
    if (complete && (!relocation_transaction_permit(transaction,RELOCATION_IO_SYNC,transaction->journal_record) || fsync(descriptor) != 0)) {
        complete = relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"The commit record could not be synchronized",transaction->journal_record);
    }
    bool closed = relocation_transaction_close(transaction,descriptor,transaction->journal_record);
    complete = complete && closed && relocation_transaction_sync(transaction,transaction->journal);
    if (!complete) { (void)retract(transaction); return false; }
    transaction->decision_pending = false;
    transaction->committed = true;
    transaction->state = RELOCATION_TRANSACTION_COMMITTED;
    return true;
}
