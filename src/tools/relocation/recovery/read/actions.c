/** @file
 * @brief Reconstruct ordered inverse records and replacement snapshots without filesystem side effects.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * recovers from saved originals rather than rerunning path rewrites.
 */
#include "../internal.h"

/** Allocation remains owned by the transaction arena after any failed decode prefix. */
static void *allocate(RelocationJournalReader *reader, size_t count, size_t size) {
    if (count > SIZE_MAX/size) {
        relocation_journal_invalid(reader,"An action array exceeds the supported allocation range");
        return NULL;
    }
    void *result = source_arena_allocate(&reader->recovery->transaction.arena,count*size);
    if (result == NULL) {
        relocation_transaction_fail(&reader->recovery->transaction,RELOCATION_OUT_OF_MEMORY,"Decoded action ownership allocation failed",NULL);
    }
    return result;
}
/** Decode ownership claims only; a separate validation phase proves names and identities before use. */
RelocationTransactionSlot *relocation_journal_read_slot(RelocationJournalReader *reader) {
    RelocationTransaction *transaction = &reader->recovery->transaction;
    RelocationTransactionSlot *owned = allocate(reader,1,sizeof(*owned));
    if (owned == NULL) { return NULL; }
    *owned = (RelocationTransactionSlot){.next=transaction->slots,.created=true,.acknowledged=true};
    transaction->slots = owned;
    if (!relocation_journal_read_text(reader,&transaction->arena,&owned->directory)
        || !relocation_journal_read_text(reader,&transaction->arena,&owned->parent)
        || !relocation_journal_read_text(reader,&transaction->arena,&owned->prepared)
        || !relocation_journal_read_text(reader,&transaction->arena,&owned->backup)
        || !relocation_journal_read_metadata(reader,&owned->metadata)) { return NULL; }
    return owned;
}
/** Prepared tree rows preserve original and replacement identity independently of renamed public paths. */
static bool tree(RelocationJournalReader *reader, RelocationTransactionAction *action) {
    RelocationTransaction *transaction = &reader->recovery->transaction;
    if (!relocation_journal_read_count(reader,&action->tree_count)) { return false; }
    if (action->tree_count == 0) { return true; }
    action->tree = allocate(reader,action->tree_count,sizeof(*action->tree));
    if (action->tree == NULL) { return false; }
    for (size_t index = 0; index < action->tree_count; index++) {
        RelocationStagedFile *file = allocate(reader,1,sizeof(*file));
        if (file == NULL) { return false; }
        *file = (RelocationStagedFile){0};
        RelocationPreparedEntry *entry = &action->tree[index];
        *entry = (RelocationPreparedEntry){.file=file,.created=true};
        if (!relocation_journal_read_text(reader,&transaction->arena,&file->before)
            || !relocation_journal_read_text(reader,&transaction->arena,&file->after)
            || !relocation_journal_read_metadata(reader,&file->metadata)
            || !relocation_journal_read_text(reader,&transaction->arena,&entry->path)
            || !relocation_journal_read_metadata(reader,&entry->metadata)) { return false; }
        if (S_ISREG(file->metadata.st_mode) || S_ISLNK(file->metadata.st_mode)) {
            bool changed;
            if (!relocation_journal_read_boolean(reader,&changed)
                || (changed && !relocation_journal_read_blob(reader,file->after,&file->replacement))) { return false; }
        } else if (!S_ISDIR(file->metadata.st_mode)) {
            return relocation_journal_invalid(reader,"A prepared tree contains an unsupported physical entry type");
        }
    }
    return true;
}
/** Attempt flags are conservative: replay probes actual owned identities to resolve interruption between rename and acknowledgement. */
bool relocation_journal_read_actions(RelocationJournalReader *reader) {
    RelocationTransaction *transaction = &reader->recovery->transaction;
    transaction->recovery_slot = relocation_journal_read_slot(reader);
    if (transaction->recovery_slot == NULL) { return false; }
    size_t count;
    if (!relocation_journal_read_count(reader,&count)) { return false; }
    for (size_t index = 0; index < count; index++) {
        uintmax_t kind;
        if (!relocation_journal_read_unsigned(reader,&kind)) { return false; }
        if (kind > RELOCATION_ACTION_REPLACE) { return relocation_journal_invalid(reader,"A journal action has an unknown operation"); }
        RelocationTransactionAction *action = allocate(reader,1,sizeof(*action));
        if (action == NULL) { return false; }
        *action = (RelocationTransactionAction){.previous=transaction->last,.kind=(RelocationActionKind)kind,
            .backup_attempted=kind != RELOCATION_ACTION_CREATE,.publish_attempted=kind != RELOCATION_ACTION_REMOVE};
        if (transaction->last == NULL) { transaction->first = action; }
        else { transaction->last->next = action; }
        transaction->last = action;
        if (!relocation_journal_read_text(reader,&transaction->arena,&action->path)) { return false; }
        action->slot = relocation_journal_read_slot(reader);
        if (action->slot == NULL || !relocation_journal_read_metadata(reader,&action->original_metadata)
            || !relocation_journal_read_metadata(reader,&action->prepared_metadata)) { return false; }
        if (action->kind == RELOCATION_ACTION_REPLACE) {
            RelocationStagedFile *file = allocate(reader,1,sizeof(*file));
            if (file == NULL) { return false; }
            *file = (RelocationStagedFile){.before=action->path,.after=action->path,.metadata=action->original_metadata};
            action->file = file;
            if (!relocation_journal_read_blob(reader,action->path,&file->replacement)) { return false; }
        }
        if (!tree(reader,action)) { return false; }
    }
    return true;
}
