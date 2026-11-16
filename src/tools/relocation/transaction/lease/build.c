/** @file
 * @brief Compose source/root/endpoint/dependency leases and validate an explicit recovery location.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * covers the complete move and external edits, rather than locking only the selected source tree.
 */
#include <string.h>
#include <unistd.h>
#include "../internal.h"
#include "../../internal.h"

/** Resolve recovery storage independently; no path borrowed from temporary resolver ownership escapes. Component contract: [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
static bool recovery_directory(RelocationTransaction *transaction, const char *input) {
    RelocationPreview authority;
    relocation_preview_init(&authority,transaction->arena.allocator);
    const char *resolved = relocation_path(&authority,input,false);
    const char *owned = resolved == NULL ? NULL : source_arena_copy(&transaction->arena,resolved);
    RelocationStatus status = authority.status;
    relocation_preview_destroy(&authority);
    if (owned == NULL) {
        return relocation_transaction_fail(transaction,status == RELOCATION_OK ? RELOCATION_OUT_OF_MEMORY : status,
            "The recovery directory could not be resolved",NULL);
    }
    const RelocationPreview *preview = transaction->staging->references->preview;
    if (relocation_contains(preview->source,owned) || relocation_contains(preview->destination,owned)) {
        return relocation_transaction_fail(transaction,RELOCATION_INVALID_PATH,"Recovery storage cannot move with the selected tree",owned);
    }
    for (const RelocationInvalidation *output = transaction->staging->invalidations; output != NULL; output = output->next) {
        if (relocation_contains(output->before,owned) || relocation_contains(output->after,owned)) {
            return relocation_transaction_fail(transaction,RELOCATION_INVALID_PATH,"Recovery storage overlaps a generated output invalidation",owned);
        }
    }
    if (access(owned,W_OK|X_OK) != 0) {
        return relocation_transaction_fail(transaction,RELOCATION_READ_ONLY,"Recovery storage is not writable",owned);
    }
    transaction->recovery_directory = owned;
    return relocation_transaction_lease_directory(transaction,owned);
}
/** A future root below D is locked through its captured original directory, never through a fabricated destination. Component contract: [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
static const char *original_root(RelocationTransaction *transaction, const char *path) {
    const RelocationPreview *preview = transaction->staging->references->preview;
    if (!relocation_contains(preview->destination,path)) { return path; }
    return source_arena_join(&transaction->arena,preview->source,"",path+strlen(preview->destination));
}
/** Nonblocking acquisition cannot deadlock; a busy later directory releases the already held prefix on destruction. Component contract: [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_transaction_lease(RelocationTransaction *transaction, const char *recovery) {
    const RelocationPreview *preview = transaction->staging->references->preview;
    if (!recovery_directory(transaction,recovery)
        || !relocation_transaction_lease_directory(transaction,preview->source)
        || !relocation_transaction_lease_parent(transaction,preview->source)
        || !relocation_transaction_lease_parent(transaction,preview->destination)) { return false; }
    for (size_t index = 0; index < preview->before_root_count; index++) {
        const char *root = preview->before_roots[index].path;
        if (!relocation_transaction_lease_directory(transaction,root) || !relocation_transaction_lease_parent(transaction,root)) { return false; }
    }
    for (size_t index = 0; index < preview->after_root_count; index++) {
        const char *root = original_root(transaction,preview->after_roots[index].path);
        if (root == NULL) { return relocation_transaction_fail(transaction,RELOCATION_OUT_OF_MEMORY,"Proposed root lease allocation failed",NULL); }
        if (!relocation_transaction_lease_directory(transaction,root) || !relocation_transaction_lease_parent(transaction,root)) { return false; }
    }
    for (const RelocationStagedFile *file = transaction->staging->files; file != NULL; file = file->next) {
        if (!relocation_transaction_lease_parent(transaction,file->before)) { return false; }
    }
    for (const RelocationInvalidation *output = transaction->staging->invalidations; output != NULL; output = output->next) {
        if (!relocation_transaction_lease_parent(transaction,output->before)) { return false; }
        if (output->existed && S_ISDIR(output->metadata.st_mode)
            && !relocation_transaction_lease_directory(transaction,output->before)) { return false; }
    }
    return true;
}
