/** @file
 * @brief Prepare every selected-tree, external-edit and output-retirement inverse before publication begins.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * treats all affected paths as one operation while retaining each original on its own filesystem.
 */
#include <string.h>
#include "../internal.h"
#include "../../internal.h"

/** Source removal retains the exact whole original tree; destination creation uses a separately verified filesystem-local copy. Component contract: [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
static bool endpoints(RelocationTransaction *transaction) {
    const RelocationPreview *preview = transaction->staging->references->preview;
    const RelocationStagedFile *root = transaction->staging->files;
    while (root != NULL && strcmp(root->before,preview->source) != 0) { root = root->next; }
    if (root == NULL || !S_ISDIR(root->metadata.st_mode)) {
        return relocation_transaction_fail(transaction,RELOCATION_INVALID_PATH,"The selected tree has no captured root directory",preview->source);
    }
    RelocationTransactionAction *remove = relocation_transaction_action(transaction,RELOCATION_ACTION_REMOVE,preview->source);
    if (remove == NULL) { return false; }
    remove->file = root;
    remove->original_metadata = root->metadata;
    RelocationTransactionAction *create = relocation_transaction_action(transaction,RELOCATION_ACTION_CREATE,preview->destination);
    return create != NULL && relocation_transaction_tree(transaction,create);
}
/** Unchanged external bytes require no replacement; all external configuration and importer changes share ordinary prepared-file handling. Component contract: [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
static bool external_files(RelocationTransaction *transaction) {
    const RelocationPreview *preview = transaction->staging->references->preview;
    for (const RelocationStagedFile *file = transaction->staging->files; file != NULL; file = file->next) {
        if (relocation_contains(preview->source,file->before) || file->original == NULL || file->replacement == NULL) { continue; }
        bool changed = source_length(file->original) != source_length(file->replacement)
            || memcmp(source_bytes(file->original),source_bytes(file->replacement),source_length(file->original)) != 0;
        if (!changed) { continue; }
        RelocationTransactionAction *action = relocation_transaction_action(transaction,RELOCATION_ACTION_REPLACE,file->before);
        if (action == NULL) { return false; }
        action->file = file;
        action->original_metadata = file->metadata;
        if (!relocation_transaction_file(transaction,action->slot->prepared,file->replacement,&file->metadata,&action->prepared_metadata)
            || !relocation_transaction_sync(transaction,action->slot->directory)) { return false; }
    }
    return true;
}
/** Generated entries inside S are absent from the prepared destination; outside outputs retain originals in separate sibling slots. Component contract: [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
static bool outputs(RelocationTransaction *transaction) {
    const RelocationPreview *preview = transaction->staging->references->preview;
    for (const RelocationInvalidation *output = transaction->staging->invalidations; output != NULL; output = output->next) {
        if (!output->existed || relocation_contains(preview->source,output->before)) { continue; }
        RelocationTransactionAction *action = relocation_transaction_action(transaction,RELOCATION_ACTION_REMOVE,output->before);
        if (action == NULL) { return false; }
        action->original_metadata = output->metadata;
    }
    return true;
}
/** Recovery storage is created first and survives incomplete cleanup; every action owns its inverse before source publication. Component contract: [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_transaction_prepare(RelocationTransaction *transaction) {
    const char *journal_target = source_arena_join(&transaction->arena,transaction->recovery_directory,"/","journal");
    if (journal_target == NULL) { return relocation_transaction_fail(transaction,RELOCATION_OUT_OF_MEMORY,"Recovery journal path allocation failed",transaction->recovery_directory); }
    transaction->recovery_slot = relocation_transaction_slot(transaction,journal_target);
    if (transaction->recovery_slot == NULL) { return false; }
    transaction->journal = transaction->recovery_slot->directory;
    return relocation_transaction_intent(transaction) && endpoints(transaction) && external_files(transaction) && outputs(transaction);
}
