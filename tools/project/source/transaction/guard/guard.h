/** @file
 * @brief Hold hierarchical exclusion through exact automatic original-state reobservation.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires fresh identities, bytes and membership under held exclusive access.
 */
#ifndef AMA_SOURCE_TRANSACTION_GUARD_H
#define AMA_SOURCE_TRANSACTION_GUARD_H
#include "../lease/lease.h"
#include "../../observation/observation.h"
#include "../../observation/freshness.h"
/** Preserve exclusion, request preparation, physical observation and stale-state failures. */
typedef enum { SOURCE_GUARD_OK, SOURCE_GUARD_INVALID, SOURCE_GUARD_LEASE,
    SOURCE_GUARD_MEMORY, SOURCE_GUARD_OBSERVATION, SOURCE_GUARD_STALE } SourceGuardStatus;
/** Own fresh observations, lease descriptors and request metadata; borrow baseline
 * and its original capture through destruction. A failed prefix remains owned;
 * do not copy the owner. valid never certifies final-plan or journal authority. */
typedef struct {
    SourceArena arena;
    SourceTransactionLease lease;
    SourceObservedImage fresh;
    const SourceObservedImage *baseline;
    SourceFreshnessIssue issue;
    SourceFreshnessStatus freshness_status;
    SourceGuardStatus status;
    bool attempted, valid;
} SourceTransactionGuard;
/** Initialize all children without host access. */
void source_transaction_guard_init(SourceTransactionGuard *, SourceAllocator);
/** Retire fresh observations before releasing exclusion; return first lease close
 * error. Keep the guard alive through publication or completed rollback. */
int source_transaction_guard_destroy(SourceTransactionGuard *);
/** Acquire once over a valid sealed original baseline, then reconstruct its exact
 * observation demands: content rows request content, complete directories request
 * children, other known rows request metadata; unavailable ignored rows remain
 * unavailable. Complete ancestor listings also preserve previously proved absence.
 * Rebuild through the common original observer and compare exact original facts.
 * Any new importer/companion or changed selected inputs refuses at observation;
 * ignored membership, opaque bytes/identity and precision changes refuse at compare.
 * No baseline replacement, source writes, execution or fallback reads occur.
 * Retain exclusion even on a failed recheck until destroy. Finite image scans,
 * the existing lease/observer/freshness bounds and O(N) request scratch bound work
 * and memory. Caller supplies the same immutable original capture and holds the
 * valid guard through the entire apply/rollback protocol.
 */
SourceGuardStatus source_transaction_guard_acquire(SourceTransactionGuard *, const SourceObservedImage *, SourceLeaseControl);
#endif
