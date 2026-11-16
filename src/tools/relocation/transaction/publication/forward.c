/** @file
 * @brief Publish prepared moves and replacements only after each inverse intent has been recorded in memory.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * retains original trees/files in same-filesystem slots until commit.
 */
#include "../internal.h"

/** A rename refusal cannot lose its inverse: attempted flags precede the call and rollback inspects actual path identities. Component contract: [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_transaction_publish(RelocationTransaction *transaction) {
    for (RelocationTransactionAction *action = transaction->first; action != NULL; action = action->next) {
        if (action->kind != RELOCATION_ACTION_CREATE) {
            if (!relocation_transaction_original(transaction,action)) { return false; }
            action->backup_attempted = true;
            if (!relocation_transaction_rename(transaction,RELOCATION_IO_BACKUP,action->path,action->slot->backup)) { return false; }
            action->saved = true;
        }
        if (action->kind != RELOCATION_ACTION_REMOVE) {
            if (relocation_transaction_probe(transaction,action->slot->prepared,&action->prepared_metadata) != RELOCATION_PATH_EXPECTED) {
                return relocation_transaction_fail(transaction,RELOCATION_CHANGED_INPUT,"A private prepared replacement changed before publication",action->slot->prepared);
            }
            action->publish_attempted = true;
            if (!relocation_transaction_rename(transaction,RELOCATION_IO_PUBLISH,action->slot->prepared,action->path)) { return false; }
            action->published = true;
        }
    }
    return true;
}
