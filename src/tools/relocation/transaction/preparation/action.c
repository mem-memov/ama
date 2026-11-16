/** @file
 * @brief Retain ordered inverses before any corresponding source mutation is possible.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * covers selected-tree movement, external replacements and generated-output retirement together.
 */
#include "../internal.h"
/** Link initialized state before slot creation so every failed preparation prefix remains discoverable. */
RelocationTransactionAction *relocation_transaction_action(RelocationTransaction *transaction, RelocationActionKind kind, const char *path) {
    RelocationTransactionAction *action = source_arena_allocate(&transaction->arena,sizeof(*action));
    if (action == NULL) {
        relocation_transaction_fail(transaction,RELOCATION_OUT_OF_MEMORY,"Relocation action ownership allocation failed",path);
        return NULL;
    }
    *action = (RelocationTransactionAction){.previous=transaction->last,.kind=kind,.path=path};
    if (transaction->last == NULL) { transaction->first = action; }
    else { transaction->last->next = action; }
    transaction->last = action;
    action->slot = relocation_transaction_slot(transaction,path);
    return action->slot == NULL ? NULL : action;
}
