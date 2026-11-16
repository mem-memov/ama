/** @file
 * @brief Prepare the entire destination tree on its own filesystem, preserving opaque entries and exact staged Ama bytes.
 * [TOOL-04](../../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * retains the original source tree across filesystem boundaries until complete verified publication.
 */
#include <stdint.h>
#include <string.h>
#include "../../internal.h"
#include "../../../internal.h"

/** T110's physical inventory is breadth-first; parents are retained before their children, independently of source activation. Component contract: [TOOL-04](../../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
static bool records(RelocationTransaction *transaction, RelocationTransactionAction *action) {
    const RelocationPreview *preview = transaction->staging->references->preview;
    size_t count = 0;
    for (const RelocationStagedFile *file = transaction->staging->files; file != NULL; file = file->next) {
        if (relocation_contains(preview->source,file->before) && !relocation_transaction_invalidated(transaction->staging,file->after)) { count++; }
    }
    if (count == 0 || count > SIZE_MAX/sizeof(RelocationPreparedEntry)) {
        return relocation_transaction_fail(transaction,count == 0 ? RELOCATION_INVALID_PATH : RELOCATION_OUT_OF_MEMORY,
            "The prepared tree inventory is empty or too large",preview->source);
    }
    action->tree = source_arena_allocate(&transaction->arena,count*sizeof(*action->tree));
    if (action->tree == NULL) { return relocation_transaction_fail(transaction,RELOCATION_OUT_OF_MEMORY,"Prepared tree record allocation failed",preview->source); }
    for (const RelocationStagedFile *file = transaction->staging->files; file != NULL; file = file->next) {
        if (!relocation_contains(preview->source,file->before) || relocation_transaction_invalidated(transaction->staging,file->after)) { continue; }
        const char *path = source_arena_join(&transaction->arena,action->slot->prepared,"",file->after+strlen(preview->destination));
        if (path == NULL) { return relocation_transaction_fail(transaction,RELOCATION_OUT_OF_MEMORY,"Prepared tree path allocation failed",file->after); }
        action->tree[action->tree_count++] = (RelocationPreparedEntry){.file=file,.path=path};
    }
    return true;
}
/** Only private paths are created; directory attributes stay writable until every descendant has been copied. Component contract: [TOOL-04](../../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
static bool entry(RelocationTransaction *transaction, RelocationPreparedEntry *prepared) {
    const RelocationStagedFile *file = prepared->file;
    if (S_ISDIR(file->metadata.st_mode)) {
        if (!relocation_transaction_permit(transaction,RELOCATION_IO_DIRECTORY,prepared->path) || mkdir(prepared->path,0700) != 0) {
            return relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"A private destination directory could not be created",prepared->path);
        }
        prepared->created = true;
        return true;
    }
    bool complete = S_ISREG(file->metadata.st_mode)
        ? relocation_transaction_file(transaction,prepared->path,file->replacement,&file->metadata,&prepared->metadata)
        : relocation_transaction_link(transaction,prepared->path,file->replacement,&file->metadata,&prepared->metadata);
    if (complete) { prepared->created = true; }
    return complete;
}
/** Reverse parent-before-child order finalizes every directory after all of its descendants; no recursive C traversal is used. Component contract: [TOOL-04](../../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_transaction_tree(RelocationTransaction *transaction, RelocationTransactionAction *action) {
    if (!records(transaction,action)) { return false; }
    for (size_t index = 0; index < action->tree_count; index++) {
        if (!entry(transaction,&action->tree[index])) { return false; }
    }
    for (size_t index = action->tree_count; index != 0; index--) {
        RelocationPreparedEntry *prepared = &action->tree[index-1];
        if (S_ISDIR(prepared->file->metadata.st_mode)
            && !relocation_transaction_directory(transaction,prepared->path,&prepared->file->metadata,&prepared->metadata)) { return false; }
    }
    action->prepared_metadata = action->tree[0].metadata;
    return relocation_transaction_sync(transaction,action->slot->directory);
}
