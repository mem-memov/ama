/** @file
 * @brief Verify every physical parent component before recovery acquires mutation authority.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * rejects symlink redirects and changed path topology instead of following them during replay.
 */
#include <string.h>
#include "internal.h"
#include "../internal.h"

/** Resolution borrows a temporary owner; the lease retains the journal-owned canonical spelling only. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_recovery_directory(RelocationTransaction *transaction, const char *path) {
    RelocationPreview preview;
    relocation_preview_init(&preview,transaction->arena.allocator);
    const char *actual = relocation_path(&preview,path,false);
    bool complete = actual != NULL && strcmp(actual,path) == 0;
    RelocationStatus status = preview.status;
    relocation_preview_destroy(&preview);
    if (!complete) {
        return relocation_transaction_fail(transaction,status == RELOCATION_OUT_OF_MEMORY ? status : RELOCATION_CHANGED_INPUT,
            "A recovery mutation parent is missing, redirected or no longer canonical",path);
    }
    return relocation_transaction_lease_directory(transaction,path);
}
