/** @file
 * @brief Persist action inverses, private slot identities and exact changed replacement bytes for recovery.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * preserves originals and reconstructible prepared state instead of applying source edits again.
 */
#include <string.h>
#include "format.h"

/** Slot ownership is recorded independently from the source/prepared objects later renamed into it. Component contract: [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_journal_slot(RelocationJournalWriter *writer, const RelocationTransactionSlot *owned) {
    return relocation_journal_text(writer,owned->directory) && relocation_journal_text(writer,owned->parent)
        && relocation_journal_text(writer,owned->prepared) && relocation_journal_text(writer,owned->backup)
        && relocation_journal_metadata(writer,&owned->metadata);
}
/** Unchanged opaque bytes remain authoritative in the saved original tree; changed Ama/configuration bytes are stored explicitly. Component contract: [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
static bool tree(RelocationJournalWriter *writer, const RelocationTransactionAction *action) {
    if (!relocation_journal_unsigned(writer,action->tree_count)) { return false; }
    for (size_t index = 0; index < action->tree_count; index++) {
        const RelocationPreparedEntry *entry = &action->tree[index];
        const RelocationStagedFile *file = entry->file;
        if (!relocation_journal_text(writer,file->before) || !relocation_journal_text(writer,file->after)
            || !relocation_journal_metadata(writer,&file->metadata) || !relocation_journal_text(writer,entry->path)
            || !relocation_journal_metadata(writer,&entry->metadata)) { return false; }
        if (S_ISREG(file->metadata.st_mode) || S_ISLNK(file->metadata.st_mode)) {
            bool changed = source_length(file->original) != source_length(file->replacement)
                || memcmp(source_bytes(file->original),source_bytes(file->replacement),source_length(file->original)) != 0;
            if (!relocation_journal_unsigned(writer,changed) || (changed && !relocation_journal_blob(writer,file->replacement))) { return false; }
        }
    }
    return true;
}
/** Every forward action has a saved inverse and source/published identity before the first mutation can occur. Component contract: [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_journal_actions(RelocationJournalWriter *writer) {
    const RelocationTransaction *transaction = writer->transaction;
    if (!relocation_journal_slot(writer,transaction->recovery_slot)) { return false; }
    size_t count = 0;
    for (const RelocationTransactionAction *action = transaction->first; action != NULL; action = action->next) { count++; }
    if (!relocation_journal_unsigned(writer,count)) { return false; }
    for (const RelocationTransactionAction *action = transaction->first; action != NULL; action = action->next) {
        if (!relocation_journal_unsigned(writer,action->kind) || !relocation_journal_text(writer,action->path)
            || !relocation_journal_slot(writer,action->slot) || !relocation_journal_metadata(writer,&action->original_metadata)
            || !relocation_journal_metadata(writer,&action->prepared_metadata)) { return false; }
        if (action->kind == RELOCATION_ACTION_REPLACE && !relocation_journal_blob(writer,action->file->replacement)) { return false; }
        if (!tree(writer,action)) { return false; }
    }
    return true;
}
