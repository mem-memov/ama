/** @file
 * @brief Own checked collection occurrences independently of referenced targets.
 * [LIST-03](../../../../spec/language/lists.md#list-03) requires compact order
 * indices and stable records; [LIST-13](../../../../spec/language/lists.md#list-13)
 * requires saved generations and collection incarnations that never revive.
 */
#ifndef AMA_RUNTIME_LIST_MEMBERSHIP_H
#define AMA_RUNTIME_LIST_MEMBERSHIP_H
#include "runtime/metadata/identity.h"
#include "source/memory/allocator.h"

/** Full context-local occurrence identity, never reconstructed from a saved index. */
typedef struct {
    AmaHandle owner;
    size_t field;
    uint64_t incarnation;
    size_t record;
    uint64_t generation;
} AmaMembership;

/** Stable record; target may be empty, stale or duplicated without merging entries. */
typedef struct {
    AmaHandle target;
    size_t position;
    size_t next_free;
    uint64_t generation;
    bool live;
} AmaMembershipRecord;

/** Metadata-only ordered collection. The caller holds exact list authority and
 * validates owner liveness. Records live in fixed blocks; only the directory and
 * size_t order entries move. No target lifecycle, acquisition or cursor is owned.
 */
typedef struct {
    SourceAllocator allocator;
    AmaHandle owner;
    size_t field;
    uint64_t incarnation;
    uint64_t counter_limit;
    size_t *order;
    size_t count;
    size_t capacity;
    AmaMembershipRecord **blocks;
    size_t block_count;
    size_t directory_capacity;
    size_t free_head;
} AmaMembershipList;

/** Initialize a new owner/field lifetime, with nonzero testable counter limit.
 * Do not reinitialize the same live namespace: use reset to advance incarnation.
 * Output remains untouched on invalid arguments; no allocation occurs.
 */
AmaStatus ama_membership_init(AmaMembershipList *, SourceAllocator, AmaHandle owner,
    size_t field, uint64_t counter_limit);
/** Free metadata only after owner retirement or otherwise preserved namespace
 * history. Never destroy targets; no borrowed record survives this call.
 */
void ama_membership_dispose(AmaMembershipList *);
/** Reconstruct a complete identity from a current protected order position.
 * Outputs are published only on success. No target-liveness test is performed.
 */
AmaStatus ama_membership_at(const AmaMembershipList *, size_t, AmaMembership *, AmaHandle *);
/** Check the complete saved identity in O(1), publishing a stable borrowed record
 * only on success. Retired occurrences cannot bind to a reused record.
 */
AmaStatus ama_membership_lookup(const AmaMembershipList *, AmaMembership, const AmaMembershipRecord **);
/** Prepare all allocation before publishing one new occurrence at position <= n.
 * Failure preserves order/identities, though private spare capacity may grow.
 */
AmaStatus ama_membership_insert(AmaMembershipList *, size_t, AmaHandle, AmaMembership *);
/** Retire a checked range and correct suffix positions without target callbacks.
 * Zero count at n succeeds. Generation exhaustion permanently retires that slot.
 */
AmaStatus ama_membership_remove(AmaMembershipList *, size_t, size_t);
/** Replace a reference occurrence atomically at unchanged position/length.
 * Identical full handles are a no-op. Preparation failure preserves the old entry.
 */
AmaStatus ama_membership_replace(AmaMembershipList *, size_t, AmaHandle, AmaMembership *);
/** Exchange current occurrences and positions; preserve identities and targets. */
AmaStatus ama_membership_swap(AmaMembershipList *, size_t, size_t);
/** Publish an empty fresh incarnation, invalidating all old identities. Exhaustion
 * preserves the entire collection. Already empty reset still advances incarnation.
 */
AmaStatus ama_membership_reset(AmaMembershipList *);
#endif
