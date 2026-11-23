/** @file
 * @brief Keep directory traversal, lock ordering and namespace verification separate.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires retained access authority before freshness and source mutation.
 */
#ifndef AMA_SOURCE_TRANSACTION_LEASE_INTERNAL_H
#define AMA_SOURCE_TRANSACTION_LEASE_INTERNAL_H
#include "lease.h"
/** Own descriptor/path/name; parent is another stable arena row or NULL for root.
 * Alias paths may have separate descriptors, but only one inode representative
 * acquires the strongest requested shared/exclusive lock. */
typedef struct SourceLeaseDirectory {
    struct SourceLeaseDirectory *next, *parent;
    const char *path, *name;
    struct stat metadata;
    int descriptor;
    bool exclusive;
} SourceLeaseDirectory;
/** Record typed first host/cancellation witness without changing acquired state. */
SourceLeaseStatus source_lease_fail(SourceTransactionLease *, SourceLeaseStatus, const char *, int);
/** Invoke optional synchronous cancellation/progress hook before a host operation. */
bool source_lease_permit(SourceTransactionLease *, SourceLeaseOperation, const char *);
/** Retain complete root walks and certify original exact root identities. */
SourceLeaseStatus source_lease_walk(SourceTransactionLease *);
/** Union inode lock demands, acquire nonblocking in deterministic order and
 * revalidate descriptor ancestry. Failed prefixes remain lease-owned. */
SourceLeaseStatus source_lease_lock(SourceTransactionLease *);
#endif
