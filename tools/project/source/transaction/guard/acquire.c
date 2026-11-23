/** @file
 * @brief Derive original observation precision and recheck it under a held hierarchical lease.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * rejects stale previews without upgrading unknown ignored entries or choosing a
 * newer baseline that happens to compile.
 */
#include <stdint.h>
#include "guard.h"
SourceGuardStatus source_transaction_guard_acquire(SourceTransactionGuard *guard,
    const SourceObservedImage *baseline, SourceLeaseControl control) {
    if (!guard || guard->attempted) { return SOURCE_GUARD_INVALID; }
    guard->attempted = true;
    if (!baseline || !baseline->valid || !baseline->sealed || !baseline->original || !baseline->original->valid) { return guard->status = SOURCE_GUARD_INVALID; }
    guard->baseline = baseline;
    if (source_transaction_lease_acquire(&guard->lease, &baseline->image, control) != SOURCE_LEASE_OK) { return guard->status = SOURCE_GUARD_LEASE; }
    size_t count = 0;
    for (size_t i = 0; i < baseline->image.count; i++) { if (baseline->image.entries[i].metadata_captured) { count++; } }
    if (count > SIZE_MAX / sizeof(SourceObservationRequest)) { return guard->status = SOURCE_GUARD_MEMORY; }
    SourceObservationRequest *requests = count ? source_arena_allocate(&guard->arena, count * sizeof(*requests)) : NULL;
    if (count && !requests) { return guard->status = SOURCE_GUARD_MEMORY; }
    size_t used = 0;
    for (size_t i = 0; i < baseline->image.count; i++) {
        const SourceCatalogImageEntry *row = &baseline->image.entries[i];
        if (!row->metadata_captured) { continue; }
        requests[used++] = (SourceObservationRequest){row->path,
            row->content ? SOURCE_OBSERVE_CONTENT : row->children_complete ? SOURCE_OBSERVE_CHILDREN : SOURCE_OBSERVE_METADATA, false};
    }
    if (source_observed_image_build(&guard->fresh, baseline->original, baseline->image.bounds, requests, count) != SOURCE_OBSERVATION_OK) { return guard->status = SOURCE_GUARD_OBSERVATION; }
    guard->freshness_status = source_observation_freshness(&baseline->image, &guard->fresh.image, &guard->issue);
    if (guard->freshness_status != SOURCE_FRESH_OK) { return guard->status = SOURCE_GUARD_STALE; }
    guard->valid = true; return guard->status = SOURCE_GUARD_OK;
}
