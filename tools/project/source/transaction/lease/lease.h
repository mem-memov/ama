/** @file
 * @brief Lease affected roots exclusively while sharing their directory ancestors.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires exclusive application access before original-state revalidation.
 * No lock files alter the captured source tree or its observed membership.
 */
#ifndef AMA_SOURCE_TRANSACTION_LEASE_H
#define AMA_SOURCE_TRANSACTION_LEASE_H
#include "../../image/image.h"
#include "source/memory/arena.h"
struct SourceLeaseDirectory;
/** Cancellation/progress observations occur before operations, never during cleanup. */
typedef enum { SOURCE_LEASE_OPEN, SOURCE_LEASE_STAT, SOURCE_LEASE_LOCK, SOURCE_LEASE_VERIFY } SourceLeaseOperation;
/** Borrow callback/context only through synchronous acquisition. */
typedef struct { void *context; bool (*permit)(void *, SourceLeaseOperation, const char *); } SourceLeaseControl;
/** Preserve contention, stale original root and ordinary host/resource failures separately. */
typedef enum { SOURCE_LEASE_OK, SOURCE_LEASE_INVALID, SOURCE_LEASE_MEMORY,
    SOURCE_LEASE_IO, SOURCE_LEASE_CHANGED, SOURCE_LEASE_BUSY, SOURCE_LEASE_CANCELLED } SourceLeaseStatus;
/** Own directory descriptors and arena metadata, borrow immutable original roots.
 * Failed acquisition retains its entire owned prefix until destroy. acquired
 * certifies cooperative hierarchical access only, never input freshness.
 */
typedef struct {
    SourceArena arena;
    struct SourceLeaseDirectory *directories;
    SourceLeaseControl control;
    const SourceCatalogImage *original;
    const char *failed_path;
    int system_error;
    SourceLeaseStatus status;
    bool attempted, acquired;
} SourceTransactionLease;
/** Begin a lease owner without opening or locking anything. */
void source_transaction_lease_init(SourceTransactionLease *, SourceAllocator);
/** Close every owned descriptor once, releasing all locks, then retire metadata.
 * Return the first close errno (zero normally); do not blindly retry close.
 * No callback can prevent cleanup; source paths and bytes remain untouched. */
int source_transaction_lease_destroy(SourceTransactionLease *);
/** Acquire once over a valid original image's exact physical observation boundaries,
 * independently of semantic module roots. A stable parent of a changed root
 * receives exclusion without becoming a catalog identity root.
 * Walk absolute normalized components with no-follow descriptor-relative opens;
 * retain every ancestor and verify original boundary device/inode/mode. Union actual
 * directory inode keys, request shared ancestors and exclusive roots, and use a
 * deterministic device/inode order with nonblocking locks. Thus disjoint roots
 * can coexist while overlapping roots and ancestor namespace operations conflict.
 * After locking, revalidate every parent/name relation through retained descriptors.
 *
 * All cooperating source operations must use this hierarchy, while init's existing
 * exclusive parent lease conflicts with descendant shared-ancestor leases.
 * Other writers must honor exclusive application access; advisory locks cannot
 * restrain unrelated programs. The caller must still reobserve the entire original
 * state under the held lease and hold it through publication/rollback.
 * No creation, source mutation, hidden wait or compiler execution. Failures retain
 * typed evidence and owned descriptors until destroy; never publish acquired.
 * Finite path, directory and original-row scans terminate apart from host calls;
 * O(D² + D*R + bytes) comparisons and O(D + path bytes) storage for D directories,
 * R original rows. Borrow image through destruction, including failed diagnostics.
 */
SourceLeaseStatus source_transaction_lease_acquire(SourceTransactionLease *, const SourceCatalogImage *, SourceLeaseControl);
#endif
