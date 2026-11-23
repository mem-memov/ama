/** @file
 * @brief Join original-root admission, no-follow traversal and hierarchical lease acquisition.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * separates exclusive access from the later original-state freshness certificate.
 */
#include "internal.h"
SourceLeaseStatus source_transaction_lease_acquire(SourceTransactionLease *lease,
    const SourceCatalogImage *original, SourceLeaseControl control) {
    if (!lease || lease->attempted) { return SOURCE_LEASE_INVALID; }
    lease->attempted = true;
    if (!original || !original->valid || !original->root_count || !original->roots) { return lease->status = SOURCE_LEASE_INVALID; }
    lease->original = original; lease->control = control;
    lease->status = source_lease_walk(lease);
    if (lease->status == SOURCE_LEASE_OK) { lease->status = source_lease_lock(lease); }
    lease->acquired = lease->status == SOURCE_LEASE_OK; return lease->status;
}
