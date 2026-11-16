/** @file
 * @brief Publish or restore only into an absent path, atomically refusing an unexpected existing destination.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * forbids overwriting changed inputs during publication or rollback.
 */
#define _GNU_SOURCE
#include <fcntl.h>
#include <stdio.h>
#include "../internal.h"
/** Linux no-replace rename protects files and directories without a check-then-overwrite race. */
bool relocation_transaction_rename(RelocationTransaction *transaction, RelocationIOOperation operation,
    const char *source, const char *destination) {
    return relocation_transaction_permit(transaction,operation,destination)
        && (renameat2(AT_FDCWD,source,AT_FDCWD,destination,RENAME_NOREPLACE) == 0
            || relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"An exclusive relocation rename failed",destination));
}
