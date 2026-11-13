/**
 * @file
 * @brief Separate context schema ownership, registry admission and private cleanup phases.
 *
 * [RT-01](../../../spec/implementation/runtime.md#rt-01) gives every context its own mutable
 * pools/constants. [RT-10](../../../spec/implementation/runtime.md#rt-10) requires prefix cleanup.
 */
#ifndef AMA_RUNTIME_CONTEXT_INTERNAL_H
#define AMA_RUNTIME_CONTEXT_INTERNAL_H
#include <stdatomic.h>
#include "context.h"
#include "runtime/lifecycle/errors.h"

/** Registry-owned independent runtime state. Descriptors/storage arrays belong to arena;
 * pools/constants own their separate allocations. active pins the record during a linear lease.
 */
struct AmaContext {
    AmaContext *next;
    uint64_t identity;
    _Atomic bool active;
    SourceArena arena;
    AmaWorld world;
    AmaProgram program;
    AmaRuntimeMetadata control;
    AmaStatus status;
    void (*report)(void *, AmaStatus);
    void *report_context;
};

/** Issue one nonzero identity from a monotone atomic clock with an immutable positive limit.
 * Output changes only on success. Strong CAS failures imply strictly newer observations;
 * exhaustion never resets or wraps the clock. Tests use a private reduced-width clock.
 */
AmaStatus ama_context_identity_next(_Atomic uint64_t *clock, uint64_t limit, uint64_t *identity);
/** Allocate a name in this runtime image's host identity namespace. The sole shared counter
 * is identity-allocation bookkeeping, never mutable Ama pool, constant or execution state.
 */
AmaStatus ama_context_identity_issue(uint64_t *identity);
/** Validate cheap schema bounds/identity uniqueness before constructors; metadata remains borrowed. */
AmaStatus ama_context_validate(const AmaContextDefinition *definition);
/** Allocate independent descriptor/storage arrays and construct shared values in a private record.
 * Failure leaves the complete initialized prefix registered for ama_context_cleanup.
 */
AmaStatus ama_context_prepare(AmaContext *context, const AmaContextDefinition *definition,
    SourceAllocator allocator);
/** Terminally clean every top-level owned tree, constants and pool/arena allocations.
 * The caller holds exclusive authority and has already removed any published identity.
 */
AmaStatus ama_context_cleanup(AmaContext *context, AmaStatus primary);
/** Lock only registry metadata, rejecting a missing/uninitialized C owner. No hooks run here. */
AmaStatus ama_context_registry_lock(AmaContextRegistry *registry);
/** Unlock a successfully held registry metadata gate; caller ensures its lifetime is pinned. */
void ama_context_registry_unlock(AmaContextRegistry *registry);
/** Find an exact live record under the registry metadata gate; absent/retired IDs return null. */
AmaContext *ama_context_find(AmaContextRegistry *registry, uint64_t identity);
/** Verify a well-formed owned lease and return its pinned record, or null for an empty lease. */
AmaContext *ama_context_leased(const AmaContextLease *lease);
#endif
