/**
 * @file
 * @brief Serialize explicit registry membership without holding a gate across native code.
 *
 * [PAR-01](../../../../spec/language/async-and-parallelism.md#par-01) allows independent
 * contexts to execute together. This short host metadata gate owns no Ama execution state.
 */
#include <threads.h>
#include "../internal.h"

/** Wait only for finite metadata critical sections, yielding to the host between attempts.
 * No allocator, native hook or Ama execution occurs while this gate is held; callbacks therefore
 * cannot recursively wait on their own registry critical section. Host scheduling supplies progress.
 * Component contract: [PAR-01](../../../../spec/language/async-and-parallelism.md#par-01). */
AmaStatus ama_context_registry_lock(AmaContextRegistry *registry) {
    if (registry == NULL || !registry->initialized) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    while (atomic_exchange_explicit(&registry->locked, true, memory_order_acquire)) {
        thrd_yield();
    }
    return ama_status_success();
}

/** Release membership publication to the next host metadata operation; the owner stays alive. Component contract: [PAR-01](../../../../spec/language/async-and-parallelism.md#par-01). */
void ama_context_registry_unlock(AmaContextRegistry *registry) {
    atomic_store_explicit(&registry->locked, false, memory_order_release);
}

/** Search exact immutable issued IDs while membership is protected; no candidate address escapes. Component contract: [PAR-01](../../../../spec/language/async-and-parallelism.md#par-01). */
AmaContext *ama_context_find(AmaContextRegistry *registry, uint64_t identity) {
    for (AmaContext *context = registry->first; context != NULL; context = context->next) {
        if (context->identity == identity) { return context; }
    }
    return NULL;
}

/** Initialize fresh caller-owned C storage. The identity namespace is never reset by registry
 * initialization, close, destruction or reuse of this registry's address.
 * Component contract: [PAR-01](../../../../spec/language/async-and-parallelism.md#par-01). */
AmaStatus ama_context_registry_init(AmaContextRegistry *registry, SourceAllocator allocator) {
    *registry = (AmaContextRegistry){0};
    atomic_init(&registry->locked, false);
    if (allocator.allocate == NULL || allocator.release == NULL) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    registry->allocator = allocator;
    registry->initialized = true;
    return ama_status_success();
}
