/** @file
 * @brief Retire only transaction-created sibling slots and keep the journal after any incomplete cleanup.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires recovery data and affected paths to survive failed restoration or final cleanup.
 */
#include <errno.h>
#include "../internal.h"

/** Missing already-retired slots are harmless; a different directory identity is never deleted as though owned. */
static bool slot(RelocationTransaction *transaction, RelocationTransactionSlot *owned) {
    if (!owned->created) { return true; }
    struct stat metadata;
    if (lstat(owned->directory,&metadata) != 0) {
        if (errno == ENOENT) { owned->created = false; return true; }
        return relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"A retained private slot could not be inspected",owned->directory);
    }
    if (!S_ISDIR(metadata.st_mode) || metadata.st_dev != owned->metadata.st_dev || metadata.st_ino != owned->metadata.st_ino) {
        return relocation_transaction_fail(transaction,RELOCATION_CHANGED_INPUT,"A retained private slot no longer has its owned identity",owned->directory);
    }
    if (!relocation_transaction_remove(transaction,owned->directory)) { return false; }
    owned->created = false;
    return true;
}
/** Call only after commit or complete inverse restoration; saved originals are otherwise intentionally retained. */
bool relocation_transaction_cleanup(RelocationTransaction *transaction) {
    bool complete = true;
    for (RelocationTransactionSlot *owned = transaction->slots; owned != NULL; owned = owned->next) {
        if (owned != transaction->recovery_slot && !slot(transaction,owned)) { complete = false; }
    }
    /* Flush even already-absent slots: an earlier executor may have removed the
     * directory but failed before its parent deletion became durable. */
    for (const RelocationTransactionSlot *owned = transaction->slots; owned != NULL; owned = owned->next) {
        if (owned != transaction->recovery_slot && !relocation_transaction_sync(transaction,owned->parent)) { complete = false; }
    }
    /* A committed receipt remains after backup cleanup, so an acknowledgement
     * failure or later recovery never has to infer the completed outcome. */
    if (complete && !transaction->committed && !transaction->retain_receipt && !transaction->intent_ready && transaction->recovery_slot != NULL) {
        complete = slot(transaction,transaction->recovery_slot);
    }
    return complete;
}
