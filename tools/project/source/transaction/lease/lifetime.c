/** @file
 * @brief Retire every directory descriptor on successful or failed lease prefixes.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03)
 * owns partial resources; [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * does not permit a failed operation to leave an unreported access lease.
 */
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "internal.h"
void source_transaction_lease_init(SourceTransactionLease *lease, SourceAllocator allocator) {
    memset(lease, 0, sizeof(*lease)); source_arena_init(&lease->arena, allocator);
}
int source_transaction_lease_destroy(SourceTransactionLease *lease) {
    int error = 0;
    for (SourceLeaseDirectory *row = lease->directories; row; row = row->next) {
        if (row->descriptor >= 0 && close(row->descriptor) != 0 && !error) { error = errno; }
    }
    source_arena_destroy(&lease->arena); memset(lease, 0, sizeof(*lease)); return error;
}
SourceLeaseStatus source_lease_fail(SourceTransactionLease *lease, SourceLeaseStatus status, const char *path, int error) {
    lease->failed_path = path; lease->system_error = error; return lease->status = status;
}
bool source_lease_permit(SourceTransactionLease *lease, SourceLeaseOperation operation, const char *path) {
    if (!lease->control.permit || lease->control.permit(lease->control.context, operation, path)) { return true; }
    source_lease_fail(lease, SOURCE_LEASE_CANCELLED, path, 0); return false;
}
