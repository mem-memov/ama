/**
 * @file
 * @brief Issue exact nonwrapping host context identities independently of context storage.
 *
 * [PAR-01](../../../../spec/language/async-and-parallelism.md#par-01) forbids reviving obsolete
 * context access. A runtime-image identity allocator is distinct from mutable Ama execution state.
 */
#include "../internal.h"

/** No context data lives here. Retaining one issuance frontier avoids retaining freed
 * context addresses or one tombstone allocation per retired context. Component export
 * identity additionally scopes separately compiled images under the later EMBED boundary. */
static _Atomic uint64_t last_context_identity;

/** Advance a fixed-domain clock exactly once. Each failed strong CAS observes a strictly
 * larger frontier, so contention is bounded by intervening issuances, not a guessed retry limit.
 */
AmaStatus ama_context_identity_next(_Atomic uint64_t *clock, uint64_t limit, uint64_t *identity) {
    uint64_t previous = atomic_load_explicit(clock, memory_order_relaxed);
    if (limit == 0 || previous > limit) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    while (previous != limit) {
        uint64_t next = previous + 1;
        if (atomic_compare_exchange_strong_explicit(clock, &previous, next,
                memory_order_relaxed, memory_order_relaxed)) {
            *identity = next;
            return ama_status_success();
        }
    }
    return ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0);
}

/** Consume a fresh image-local control identity, including failed factory attempts; never recycle. */
AmaStatus ama_context_identity_issue(uint64_t *identity) {
    return ama_context_identity_next(&last_context_identity, UINT64_MAX, identity);
}
