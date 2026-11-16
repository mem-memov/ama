/** @file
 * @brief Retract incomplete decision/finalization appends durably before uncommitted inverse replay.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * never restores originals while a commit could still be recovered as acknowledged.
 */
#include <fcntl.h>
#include <unistd.h>
#include "internal.h"

/** The strict decoder already proved that bytes after the prepared prefix are proper unfinished tokens. */
bool relocation_recovery_retract(RelocationRecovery *recovery) {
    RelocationTransaction *transaction = &recovery->transaction;
    if (!relocation_transaction_permit(transaction,RELOCATION_IO_JOURNAL,transaction->journal_record)) { return false; }
    int descriptor = open(transaction->journal_record,O_RDWR|O_NOFOLLOW|O_CLOEXEC);
    if (descriptor < 0) { return relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"The interrupted decision could not be reopened",transaction->journal_record); }
    bool complete = ftruncate(descriptor,transaction->journal_length) == 0;
    if (!complete) { relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"The interrupted decision could not be retracted",transaction->journal_record); }
    if (complete && (!relocation_transaction_permit(transaction,RELOCATION_IO_SYNC,transaction->journal_record) || fsync(descriptor) != 0)) {
        complete = relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"The interrupted decision retraction could not be synchronized",transaction->journal_record);
    }
    bool closed = relocation_transaction_close(transaction,descriptor,transaction->journal_record);
    return complete && closed && relocation_transaction_sync(transaction,transaction->journal);
}
