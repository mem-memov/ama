/** @file
 * @brief Own decoded recovery snapshots independently of the vanished compiler/application process.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * retains saved originals after failures rather than deleting them during object destruction.
 */
#include "internal.h"
/** Recovery begins with no filesystem authority, loaded record or report. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
void relocation_recovery_init(RelocationRecovery *recovery, SourceAllocator allocator) {
    *recovery = (RelocationRecovery){0};
    relocation_transaction_init(&recovery->transaction,allocator);
    source_manager_init(&recovery->sources,allocator);
}
/** Decoded action views expire before their payload/source owner; no filesystem cleanup is implicit. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
void relocation_recovery_destroy(RelocationRecovery *recovery) {
    relocation_transaction_destroy(&recovery->transaction);
    source_manager_destroy(&recovery->sources);
    *recovery = (RelocationRecovery){0};
}
