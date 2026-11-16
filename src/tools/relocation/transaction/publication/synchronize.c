/** @file
 * @brief Make restored public names durable before transaction cleanup removes any remaining private copy.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * retains recoverable state when rollback synchronization itself fails.
 */
#include "../internal.h"

/** Every attempted inverse's parent is flushed before cleanup; untouched preparation prefixes need no source flush. */
bool relocation_transaction_restored(RelocationTransaction *transaction) {
    for (const RelocationTransactionAction *action = transaction->first; action != NULL; action = action->next) {
        if ((action->backup_attempted || action->publish_attempted)
            && !relocation_transaction_sync(transaction,action->slot->parent)) { return false; }
    }
    return true;
}
