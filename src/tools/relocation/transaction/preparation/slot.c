/** @file
 * @brief Create owned same-filesystem sibling slots while retaining every successful prefix for cleanup.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * preserves original files in recoverable storage rather than assuming one project-wide rename.
 */
#include <stdlib.h>
#include <string.h>
#include "../internal.h"
/** Link the ownership record before creation; a later allocation error cannot lose an already created private directory. */
RelocationTransactionSlot *relocation_transaction_slot(RelocationTransaction *transaction, const char *target) {
    char *parent = source_arena_copy(&transaction->arena,target);
    RelocationTransactionSlot *slot = source_arena_allocate(&transaction->arena,sizeof(*slot));
    if (parent == NULL || slot == NULL) {
        relocation_transaction_fail(transaction,RELOCATION_OUT_OF_MEMORY,"Relocation slot ownership allocation failed",target);
        return NULL;
    }
    char *slash = strrchr(parent,'/');
    if (slash == NULL) {
        relocation_transaction_fail(transaction,RELOCATION_INVALID_PATH,"A relocation slot target is not absolute",target);
        return NULL;
    }
    if (slash == parent) { slash[1] = 0; } else { *slash = 0; }
    bool reserved = transaction->intent_ready;
    const char *directory = reserved ? relocation_transaction_slot_name(transaction,parent)
        : source_arena_join(&transaction->arena,parent,strcmp(parent,"/") == 0 ? "" : "/",".ama-relocation-XXXXXX");
    if (directory == NULL) {
        if (transaction->status == RELOCATION_OK) { relocation_transaction_fail(transaction,RELOCATION_OUT_OF_MEMORY,"Relocation slot path allocation failed",target); }
        return NULL;
    }
    *slot = (RelocationTransactionSlot){.next=transaction->slots,.directory=directory,.parent=parent};
    transaction->slots = slot;
    if (reserved) {
        slot->prepared = source_arena_join(&transaction->arena,directory,"/","prepared");
        slot->backup = source_arena_join(&transaction->arena,directory,"/","original");
        if (slot->prepared == NULL || slot->backup == NULL) {
            relocation_transaction_fail(transaction,RELOCATION_OUT_OF_MEMORY,"Relocation slot child allocation failed",directory);
            return NULL;
        }
        if (!relocation_transaction_reserve(transaction,slot)) { return NULL; }
    }
    if (!relocation_transaction_permit(transaction,RELOCATION_IO_TEMPORARY,directory)) { return NULL; }
    bool created = reserved ? mkdir(directory,0700) == 0 : mkdtemp((char *)directory) != NULL;
    if (!created) {
        relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"A private relocation slot could not be created",directory);
        return NULL;
    }
    slot->created = true;
    if (lstat(directory,&slot->metadata) != 0) {
        relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"A private relocation slot identity could not be read",directory);
        return NULL;
    }
    slot->acknowledged = true;
    if (reserved) { return relocation_transaction_own(transaction,slot) ? slot : NULL; }
    slot->prepared = source_arena_join(&transaction->arena,directory,"/","prepared");
    slot->backup = source_arena_join(&transaction->arena,directory,"/","original");
    if (slot->prepared == NULL || slot->backup == NULL) {
        relocation_transaction_fail(transaction,RELOCATION_OUT_OF_MEMORY,"Relocation slot child allocation failed",directory);
        return NULL;
    }
    return slot;
}
