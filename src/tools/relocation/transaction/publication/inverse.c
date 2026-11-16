/** @file
 * @brief Restore attempted source changes in reverse order while preserving foreign paths and saved originals.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * retains incomplete recovery data instead of overwriting conflicting state or reporting success.
 */
#include "../internal.h"

/** Withdraw only this action's prepared object; an already absent target needs no deletion or repeated rewrite. */
static bool withdraw(RelocationTransaction *transaction, RelocationTransactionAction *action) {
    if (!action->publish_attempted) { return true; }
    RelocationPathState target = relocation_transaction_probe(transaction,action->path,&action->prepared_metadata);
    if (target == RELOCATION_PATH_ABSENT) { action->published = false; return true; }
    if (target == RELOCATION_PATH_OTHER && action->kind != RELOCATION_ACTION_CREATE
        && relocation_transaction_probe(transaction,action->path,&action->original_metadata) == RELOCATION_PATH_EXPECTED) {
        action->published = false;
        return true;
    }
    if (target != RELOCATION_PATH_EXPECTED) {
        return relocation_transaction_fail(transaction,RELOCATION_CHANGED_INPUT,"A published replacement cannot be withdrawn without touching foreign state",action->path);
    }
    bool renamed = relocation_transaction_rename(transaction,RELOCATION_IO_RESTORE,action->path,action->slot->prepared);
    if (!renamed && (relocation_transaction_probe(transaction,action->path,&action->prepared_metadata) != RELOCATION_PATH_ABSENT
        || relocation_transaction_probe(transaction,action->slot->prepared,&action->prepared_metadata) != RELOCATION_PATH_EXPECTED)) { return false; }
    action->published = false;
    return true;
}
/** An original already restored is accepted; an unexpected existing path or missing original remains a recoverable conflict. */
static bool restore(RelocationTransaction *transaction, RelocationTransactionAction *action) {
    if (!action->backup_attempted) { return true; }
    RelocationPathState saved = relocation_transaction_probe(transaction,action->slot->backup,&action->original_metadata);
    RelocationPathState target = relocation_transaction_probe(transaction,action->path,&action->original_metadata);
    if (saved == RELOCATION_PATH_ABSENT && target == RELOCATION_PATH_EXPECTED) { action->saved = false; return true; }
    if (saved != RELOCATION_PATH_EXPECTED || target != RELOCATION_PATH_ABSENT) {
        return relocation_transaction_fail(transaction,RELOCATION_CHANGED_INPUT,"A saved original cannot be restored without losing or overwriting data",action->path);
    }
    bool renamed = relocation_transaction_rename(transaction,RELOCATION_IO_RESTORE,action->slot->backup,action->path);
    if (!renamed && (relocation_transaction_probe(transaction,action->slot->backup,&action->original_metadata) != RELOCATION_PATH_ABSENT
        || relocation_transaction_probe(transaction,action->path,&action->original_metadata) != RELOCATION_PATH_EXPECTED)) { return false; }
    action->saved = false;
    return true;
}
/** Continue independent inverse actions after failure; one retained original must not prevent restoration of other paths. */
bool relocation_transaction_rollback(RelocationTransaction *transaction) {
    bool complete = true;
    for (RelocationTransactionAction *action = transaction->last; action != NULL; action = action->previous) {
        if (!withdraw(transaction,action) || !restore(transaction,action)) { complete = false; }
    }
    return complete;
}
