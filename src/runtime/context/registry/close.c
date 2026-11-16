/**
 * @file
 * @brief Invalidate checked context membership before complete exclusive lifecycle cleanup.
 *
 * [RT-10](../../../../spec/implementation/runtime.md#rt-10) owns every cleanup path.
 * [PAR-01](../../../../spec/language/async-and-parallelism.md#par-01) prevents freeing any
 * context while an executor or borrowed view holds its gate.
 */
#include "../internal.h"

/** Acquire all record gates without changing Ama state. A busy record restores every earlier
 * temporary gate before refusal, preserving complete membership and resource ownership.
 * Component contract: [RT-10](../../../../spec/implementation/runtime.md#rt-10), [PAR-01](../../../../spec/language/async-and-parallelism.md#par-01). */
static bool context_close_admit(AmaContextRegistry *registry) {
    for (AmaContext *context = registry->first; context != NULL; context = context->next) {
        bool available = false;
        if (atomic_compare_exchange_strong_explicit(&context->active, &available, true,
                memory_order_acquire, memory_order_relaxed)) { continue; }
        for (AmaContext *previous = registry->first; previous != context; previous = previous->next) {
            atomic_store_explicit(&previous->active, false, memory_order_release);
        }
        return false;
    }
    return true;
}

/** Detach all admitted records, then consume them exactly once. A cleanup pin blocks recursive
 * registry destruction until every hook returns, while stale-token checks remain available.
 * Component contract: [RT-10](../../../../spec/implementation/runtime.md#rt-10), [PAR-01](../../../../spec/language/async-and-parallelism.md#par-01). */
AmaStatus ama_context_registry_close(AmaContextRegistry *registry) {
    AmaStatus status = ama_context_registry_lock(registry);
    if (status.code != AMA_OK) { return status; }
    if (registry->pending != 0) { status = ama_status_make(AMA_BUSY, NULL, 0); }
    else if (registry->closed) { status = registry->close_status; }
    else if (!context_close_admit(registry)) { status = ama_status_make(AMA_BUSY, NULL, 0); }
    else {
        AmaContext *context = registry->first;
        SourceAllocator allocator = registry->allocator;
        registry->first = NULL;
        registry->closed = true;
        registry->pending = 1;
        ama_context_registry_unlock(registry);
        while (context != NULL) {
            AmaContext *next = context->next;
            status = ama_cleanup_primary(status, ama_context_cleanup(context, ama_status_success()));
            allocator.release(allocator.context, context);
            context = next;
        }
        (void)ama_context_registry_lock(registry);
        registry->pending = 0;
        registry->close_status = status;
    }
    ama_context_registry_unlock(registry);
    return status;
}

/** Release caller-owned registry lifetime only after close admission. The caller supplies
 * exclusive C lifetime authority; pending callbacks and live leases preserve an initialized owner.
 * Component contract: [RT-10](../../../../spec/implementation/runtime.md#rt-10), [PAR-01](../../../../spec/language/async-and-parallelism.md#par-01). */
AmaStatus ama_context_registry_destroy(AmaContextRegistry *registry) {
    if (registry == NULL || !registry->initialized) { return ama_status_success(); }
    AmaStatus status = ama_context_registry_close(registry);
    if (registry->closed && registry->pending == 0) { registry->initialized = false; }
    return status;
}
