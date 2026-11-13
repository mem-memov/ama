/**
 * @file
 * @brief Retire one checked context before running its exclusive resource cleanup.
 *
 * [PAR-01](../../../../spec/language/async-and-parallelism.md#par-01) pins active contexts.
 * [RT-10](../../../../spec/implementation/runtime.md#rt-10) keeps registry ownership alive
 * through native cleanup callbacks without serializing unrelated Ama execution.
 */
#include "../internal.h"

/** Retire one identity under membership protection, then clean native resources without
 * holding the metadata gate. A pending credit pins registry lifetime through callbacks.
 */
AmaStatus ama_context_dispose(AmaContextRegistry *registry, uint64_t identity) {
    AmaStatus status = ama_context_registry_lock(registry);
    if (status.code != AMA_OK) { return status; }
    AmaContext **link = &registry->first;
    while (*link != NULL && (*link)->identity != identity) { link = &(*link)->next; }
    AmaContext *context = *link;
    if (identity == 0 || context == NULL) {
        ama_context_registry_unlock(registry);
        return ama_status_make(identity == 0 ? AMA_EMPTY_REFERENCE : AMA_STALE_REFERENCE, NULL, 0);
    }
    bool available = false;
    if (registry->pending == SIZE_MAX) { status = ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0); }
    else if (!atomic_compare_exchange_strong_explicit(&context->active, &available, true,
            memory_order_acquire, memory_order_relaxed)) { status = ama_status_make(AMA_BUSY, NULL, 0); }
    if (status.code != AMA_OK) { ama_context_registry_unlock(registry); return status; }
    *link = context->next;
    registry->pending++;
    SourceAllocator allocator = registry->allocator;
    ama_context_registry_unlock(registry);
    status = ama_context_cleanup(context, ama_status_success());
    allocator.release(allocator.context, context);
    (void)ama_context_registry_lock(registry);
    registry->pending--;
    ama_context_registry_unlock(registry);
    return status;
}
