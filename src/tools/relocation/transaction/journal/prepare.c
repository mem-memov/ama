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
#include "../internal.h"

/** Decimal lengths frame arbitrary path bytes, including newlines, rather than interpreting them as record syntax. */
static bool number(RelocationTransaction *transaction, int descriptor, uintmax_t value) {
    char bytes[3*sizeof(uintmax_t)+3];
    int length = snprintf(bytes,sizeof(bytes),"%ju\n",value);
    return (length > 0 && (size_t)length < sizeof(bytes)
        && relocation_transaction_write(transaction,descriptor,transaction->journal_record,(const unsigned char *)bytes,(size_t)length))
        || relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"A prepared journal integer could not be written",transaction->journal_record);
}
/** A path's exact byte count precedes its bytes; the following record starts immediately after those bytes. */
static bool path(RelocationTransaction *transaction, int descriptor, const char *value) {
    size_t length = strlen(value);
    return number(transaction,descriptor,length)
        && relocation_transaction_write(transaction,descriptor,transaction->journal_record,(const unsigned char *)value,length);
}
/** The prepared inventory identifies both original and newly prepared physical objects; replay is supplied by T114. */
static bool inventory(RelocationTransaction *transaction, int descriptor) {
    static const unsigned char header[] = "AMA-RELOCATION-PLAN-1\n";
    if (!relocation_transaction_write(transaction,descriptor,transaction->journal_record,header,sizeof(header)-1)) { return false; }
    size_t count = 0;
    for (const RelocationTransactionAction *action = transaction->first; action != NULL; action = action->next) { count++; }
    if (!number(transaction,descriptor,count)) { return false; }
    for (const RelocationTransactionAction *action = transaction->first; action != NULL; action = action->next) {
        if (!number(transaction,descriptor,action->kind) || !path(transaction,descriptor,action->path)
            || !path(transaction,descriptor,action->slot->directory) || !path(transaction,descriptor,action->slot->prepared)
            || !path(transaction,descriptor,action->slot->backup)
            || !number(transaction,descriptor,(uintmax_t)action->original_metadata.st_dev)
            || !number(transaction,descriptor,(uintmax_t)action->original_metadata.st_ino)
            || !number(transaction,descriptor,(uintmax_t)action->original_metadata.st_mode)
            || !number(transaction,descriptor,(uintmax_t)action->prepared_metadata.st_dev)
            || !number(transaction,descriptor,(uintmax_t)action->prepared_metadata.st_ino)
            || !number(transaction,descriptor,(uintmax_t)action->prepared_metadata.st_mode)) { return false; }
    }
    return relocation_transaction_write(transaction,descriptor,transaction->journal_record,(const unsigned char *)"END\n",4);
}
/** Complete inventory and all private slot names are synchronized before the first original backup rename. */
bool relocation_transaction_journal(RelocationTransaction *transaction) {
    transaction->journal_record = source_arena_join(&transaction->arena,transaction->journal,"/","plan");
    if (transaction->journal_record == NULL) {
        return relocation_transaction_fail(transaction,RELOCATION_OUT_OF_MEMORY,"Prepared journal record path allocation failed",transaction->journal);
    }
    if (!relocation_transaction_permit(transaction,RELOCATION_IO_JOURNAL,transaction->journal_record)) { return false; }
    int descriptor = open(transaction->journal_record,O_RDWR|O_CREAT|O_EXCL|O_NOFOLLOW|O_CLOEXEC,0600);
    if (descriptor < 0) { return relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"The prepared journal could not be created",transaction->journal_record); }
    bool complete = inventory(transaction,descriptor);
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
    return true;
}
