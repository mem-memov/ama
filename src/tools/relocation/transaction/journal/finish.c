/** @file
 * @brief Persist terminal outcome receipts so repeated recovery never touches a later edited workspace.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * makes completed recovery repeatable without inferring success from current source paths.
 */
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "../internal.h"

/** Truncating an interrupted final marker preserves the complete prepared prefix and acknowledged commit decision. Component contract: [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_transaction_finish(RelocationTransaction *transaction) {
    if (!transaction->retain_receipt) { return true; }
    if (!relocation_transaction_permit(transaction,RELOCATION_IO_JOURNAL,transaction->journal_record)) { return false; }
    int descriptor = open(transaction->journal_record,O_RDWR|O_NOFOLLOW|O_CLOEXEC);
    if (descriptor < 0) { return relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"A completed outcome receipt could not be opened",transaction->journal_record); }
    off_t offset = transaction->journal_length;
    bool complete = true;
    if (transaction->committed) {
        intmax_t value = (intmax_t)offset;
        if (value > INTMAX_MAX-7 || (intmax_t)(off_t)(value+7) != value+7) {
            complete = relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"A completed receipt offset exceeds its supported range",transaction->journal_record);
        } else { offset = (off_t)(value+7); }
    }
    if (complete && (ftruncate(descriptor,offset) != 0 || lseek(descriptor,offset,SEEK_SET) < 0)) {
        complete = relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"A completed receipt could not be positioned",transaction->journal_record);
    }
    const char *marker = transaction->committed ? "DONE-COMMIT\n" : "DONE-ROLLBACK\n";
    if (complete) { complete = relocation_transaction_write(transaction,descriptor,transaction->journal_record,(const unsigned char *)marker,strlen(marker)); }
    if (complete && (!relocation_transaction_permit(transaction,RELOCATION_IO_SYNC,transaction->journal_record) || fsync(descriptor) != 0)) {
        complete = relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"A completed outcome receipt could not be synchronized",transaction->journal_record);
    }
    bool closed = relocation_transaction_close(transaction,descriptor,transaction->journal_record);
    return complete && closed && relocation_transaction_sync(transaction,transaction->journal);
}
