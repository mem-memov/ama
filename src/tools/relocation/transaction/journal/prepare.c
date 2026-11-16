/** @file
 * @brief Retain a framed prepared action inventory before publication can change source paths.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires saved originals and sufficient transaction state for subsequent recovery work.
 */
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "format.h"

/** Complete inventory and all private slot names are synchronized before the first original backup rename. Component contract: [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_transaction_journal(RelocationTransaction *transaction) {
    transaction->journal_record = source_arena_join(&transaction->arena,transaction->journal,"/","plan");
    if (transaction->journal_record == NULL) {
        return relocation_transaction_fail(transaction,RELOCATION_OUT_OF_MEMORY,"Prepared journal record path allocation failed",transaction->journal);
    }
    if (!relocation_transaction_permit(transaction,RELOCATION_IO_JOURNAL,transaction->journal_record)) { return false; }
    int descriptor = open(transaction->journal_record,O_RDWR|O_CREAT|O_EXCL|O_NOFOLLOW|O_CLOEXEC,0600);
    if (descriptor < 0) { return relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"The prepared journal could not be created",transaction->journal_record); }
    RelocationJournalWriter writer = {transaction,descriptor,transaction->journal_record};
    bool complete = relocation_journal_bytes(&writer,(const unsigned char *)RELOCATION_JOURNAL_MAGIC,sizeof(RELOCATION_JOURNAL_MAGIC)-1)
        && relocation_journal_report(&writer,&transaction->report) && relocation_journal_actions(&writer)
        && relocation_journal_bytes(&writer,(const unsigned char *)"END\n",4);
    if (complete) {
        struct stat metadata;
        complete = fstat(descriptor,&metadata) == 0;
        if (complete) { transaction->journal_length = metadata.st_size; }
        else { relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"The prepared journal length could not be read",transaction->journal_record); }
    }
    if (complete && (!relocation_transaction_permit(transaction,RELOCATION_IO_SYNC,transaction->journal_record) || fsync(descriptor) != 0)) {
        complete = relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"The prepared journal could not be synchronized",transaction->journal_record);
    }
    bool closed = relocation_transaction_close(transaction,descriptor,transaction->journal_record);
    if (!complete || !closed) { return false; }
    for (const RelocationTransactionSlot *slot = transaction->slots; slot != NULL; slot = slot->next) {
        if (!relocation_transaction_sync(transaction,slot->directory) || !relocation_transaction_sync(transaction,slot->parent)) { return false; }
    }
    if (!relocation_transaction_ready(transaction)) { return false; }
    transaction->retain_receipt = true;
    return true;
}
