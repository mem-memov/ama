/** @file
 * @brief Persist preparation reservations and owned identities before private copies can become nonempty.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * recovers interrupted preparation without scanning or guessing temporary directory ownership.
 */
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "format.h"

/** Complete each append and its fsync before allowing the operation that depends on that authority. Component contract: [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
static bool close_record(RelocationJournalWriter *writer, bool complete) {
    RelocationTransaction *transaction = writer->transaction;
    if (complete && (!relocation_transaction_permit(transaction,RELOCATION_IO_SYNC,transaction->intent_record) || fsync(writer->descriptor) != 0)) {
        complete = relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"Preparation intent could not be synchronized",transaction->intent_record);
    }
    struct stat metadata;
    if (complete && fstat(writer->descriptor,&metadata) != 0) {
        complete = relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"Preparation intent length could not be read",transaction->intent_record);
    }
    bool closed = relocation_transaction_close(transaction,writer->descriptor,transaction->intent_record);
    complete = complete && closed && relocation_transaction_sync(transaction,transaction->journal);
    if (complete) { transaction->intent_length = metadata.st_size; }
    return complete;
}
/** Only this transaction's private journal is opened, with no symlink traversal or overwrite of an existing initial record. Component contract: [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
static bool open_record(RelocationTransaction *transaction, RelocationJournalWriter *writer, bool create) {
    if (!relocation_transaction_permit(transaction,RELOCATION_IO_JOURNAL,transaction->intent_record)) { return false; }
    int flags = O_WRONLY|O_NOFOLLOW|O_CLOEXEC|(create ? O_CREAT|O_EXCL : O_APPEND);
    int descriptor = open(transaction->intent_record,flags,0600);
    if (descriptor < 0) { return relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"Preparation intent could not be opened",transaction->intent_record); }
    *writer = (RelocationJournalWriter){transaction,descriptor,transaction->intent_record};
    return true;
}
/** The initial root is empty until its scope and identity are durably recorded; no source mutation is possible here. Component contract: [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_transaction_intent(RelocationTransaction *transaction) {
    transaction->intent_record = source_arena_join(&transaction->arena,transaction->journal,"/","intent");
    if (transaction->intent_record == NULL) { return relocation_transaction_fail(transaction,RELOCATION_OUT_OF_MEMORY,"Preparation intent path allocation failed",transaction->journal); }
    RelocationJournalWriter writer;
    if (!open_record(transaction,&writer,true)) { return false; }
    bool complete = relocation_journal_bytes(&writer,(const unsigned char *)RELOCATION_INTENT_MAGIC,sizeof(RELOCATION_INTENT_MAGIC)-1)
        && relocation_journal_report(&writer,&transaction->report) && relocation_journal_slot(&writer,transaction->recovery_slot)
        && relocation_journal_bytes(&writer,(const unsigned char *)"BEGIN\n",6);
    if (!close_record(&writer,complete)) { return false; }
    transaction->intent_ready = true;
    return true;
}
/** Reservation is durable before mkdir; an interrupted unacknowledged reservation can only own an absent or empty directory. Component contract: [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_transaction_reserve(RelocationTransaction *transaction, const RelocationTransactionSlot *slot) {
    RelocationJournalWriter writer;
    if (!open_record(transaction,&writer,false)) { return false; }
    bool complete = relocation_journal_bytes(&writer,(const unsigned char *)"SLOT\n",5)
        && relocation_journal_text(&writer,slot->directory) && relocation_journal_text(&writer,slot->parent)
        && relocation_journal_text(&writer,slot->prepared) && relocation_journal_text(&writer,slot->backup)
        && relocation_journal_bytes(&writer,(const unsigned char *)"END-SLOT\n",9);
    return close_record(&writer,complete);
}
/** No prepared bytes or saved originals may enter the directory before this exact identity is durable. Component contract: [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_transaction_own(RelocationTransaction *transaction, const RelocationTransactionSlot *slot) {
    RelocationJournalWriter writer;
    if (!open_record(transaction,&writer,false)) { return false; }
    bool complete = relocation_journal_bytes(&writer,(const unsigned char *)"OWN\n",4)
        && relocation_journal_text(&writer,slot->directory) && relocation_journal_metadata(&writer,&slot->metadata)
        && relocation_journal_bytes(&writer,(const unsigned char *)"END-OWN\n",8);
    return close_record(&writer,complete);
}
/** Publication is authorized only after the complete plan and every prepared private directory are durable. Component contract: [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_transaction_ready(RelocationTransaction *transaction) {
    RelocationJournalWriter writer;
    if (!open_record(transaction,&writer,false)) { return false; }
    bool complete = relocation_journal_bytes(&writer,(const unsigned char *)"READY\n",6);
    return close_record(&writer,complete);
}
/** A failed preparation never changed sources; keep a terminal abort receipt after registered slots retire. Component contract: [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_transaction_abandoned(RelocationTransaction *transaction) {
    if (!transaction->intent_ready) { return true; }
    RelocationJournalWriter writer;
    if (!open_record(transaction,&writer,false)) { return false; }
    bool complete = ftruncate(writer.descriptor,transaction->intent_length) == 0;
    if (!complete) { relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"An interrupted preparation append could not be retracted",transaction->intent_record); }
    if (complete) { complete = relocation_journal_bytes(&writer,(const unsigned char *)"ABORTED\n",8); }
    return close_record(&writer,complete);
}
