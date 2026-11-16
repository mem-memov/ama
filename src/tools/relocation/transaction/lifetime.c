/** @file
 * @brief Retain transaction ownership and both primary and rollback diagnostics without allocation.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * forbids success after incomplete rollback and requires recovery data to remain.
 */
#include "internal.h"
/** Initialization creates no filesystem state and borrows only allocator authority. */
void relocation_transaction_init(RelocationTransaction *transaction, SourceAllocator allocator) {
    *transaction = (RelocationTransaction){0};
    source_arena_init(&transaction->arena,allocator);
}
/** Destruction never interprets an incomplete state as permission to delete saved originals. */
void relocation_transaction_destroy(RelocationTransaction *transaction) {
    relocation_transaction_release(transaction);
    source_arena_destroy(&transaction->arena);
    *transaction = (RelocationTransaction){0};
}
/** Cleanup records its first separate failure while retaining the original cause. */
bool relocation_transaction_fail(RelocationTransaction *transaction, RelocationStatus status, const char *message, const char *path) {
    Diagnostic *diagnostic = transaction->cleaning && transaction->error.message != NULL
        ? &transaction->cleanup_error : &transaction->error;
    if (diagnostic->message == NULL) {
        *diagnostic = (Diagnostic){.code=status == RELOCATION_OUT_OF_MEMORY ? DIAGNOSTIC_OUT_OF_MEMORY
            : status == RELOCATION_INVALID_PATH || status == RELOCATION_INVALID_REFERENCE
                ? DIAGNOSTIC_INVALID_PATH : DIAGNOSTIC_IO_FAILURE,
            .message=message,.primary={.path=path}};
    }
    if (transaction->status == RELOCATION_OK) { transaction->status = status; }
    return false;
}
