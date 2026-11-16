/**
 * @file
 * @brief Acquire context-local execution authority before exposing mutable runtime owners.
 *
 * [PAR-01](../../../spec/language/async-and-parallelism.md#par-01) allows one executor per
 * context; [EMBED-03](../../../spec/implementation/c-library.md#embed-03) separates rejected
 * host admission from accepted execution. No Ama refusal/history is changed here.
 * [PAR-02](../../../spec/language/async-and-parallelism.md#par-02) permits
 * a quiescent owner change: release/acquire publishes context storage, while the
 * host separately excludes conflicting access and checks every native affinity.
 */
#include "internal.h"

/** Resolve a numeric identity under membership protection and acquire its atomic context gate.
 * Only membership/immutable ID and the gate are touched before success; world state is not read.
 * Component contract: [PAR-01](../../../spec/language/async-and-parallelism.md#par-01), [EMBED-03](../../../spec/implementation/c-library.md#embed-03), [PAR-02](../../../spec/language/async-and-parallelism.md#par-02). */
AmaStatus ama_context_enter(AmaContextRegistry *registry, uint64_t identity, AmaContextLease *lease) {
    if (lease == NULL || lease->context != NULL || lease->registry != NULL || lease->identity != 0) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    AmaStatus status = ama_context_registry_lock(registry);
    if (status.code != AMA_OK) { return status; }
    AmaContext *context = ama_context_find(registry, identity);
    if (identity == 0 || context == NULL) {
        status = ama_status_make(identity == 0 ? AMA_EMPTY_REFERENCE : AMA_STALE_REFERENCE, NULL, 0);
    } else {
        bool available = false;
        if (!atomic_compare_exchange_strong_explicit(&context->active, &available, true,
                memory_order_acquire, memory_order_relaxed)) {
            status = ama_status_make(AMA_BUSY, NULL, 0);
        } else { *lease = (AmaContextLease){registry, context, identity}; }
    }
    ama_context_registry_unlock(registry);
    return status;
}

/** Validate only a supported linear C lease. Its gate pins record lifetime; arbitrary forged
 * lease pointers are invalid C ownership, unlike numeric checked context/instance tokens.
 * Component contract: [PAR-01](../../../spec/language/async-and-parallelism.md#par-01), [EMBED-03](../../../spec/implementation/c-library.md#embed-03), [PAR-02](../../../spec/language/async-and-parallelism.md#par-02). */
AmaContext *ama_context_leased(const AmaContextLease *lease) {
    if (lease == NULL || lease->context == NULL || lease->registry == NULL || lease->identity == 0) { return NULL; }
    AmaContext *context = lease->context;
    return context->identity == lease->identity
        && atomic_load_explicit(&context->active, memory_order_relaxed) ? context : NULL;
}

/** Consume the lease before releasing its last record access. Disposal may free the record
 * immediately after this release-store, so no context field is read afterward.
 * Component contract: [PAR-01](../../../spec/language/async-and-parallelism.md#par-01), [EMBED-03](../../../spec/implementation/c-library.md#embed-03), [PAR-02](../../../spec/language/async-and-parallelism.md#par-02). */
AmaStatus ama_context_leave(AmaContextLease *lease) {
    AmaContext *context = ama_context_leased(lease);
    if (context == NULL) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    *lease = (AmaContextLease){0};
    atomic_store_explicit(&context->active, false, memory_order_release);
    return ama_status_success();
}

/** Publish borrowed owners only while exclusive authority is live. Context diagnostic status
 * is a value snapshot; its message still follows the immutable program-lifetime ABI contract.
 * Component contract: [PAR-01](../../../spec/language/async-and-parallelism.md#par-01), [EMBED-03](../../../spec/implementation/c-library.md#embed-03), [PAR-02](../../../spec/language/async-and-parallelism.md#par-02). */
AmaStatus ama_context_inspect(const AmaContextLease *lease, AmaContextView *view) {
    AmaContext *context = ama_context_leased(lease);
    if (context == NULL || view == NULL) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    *view = (AmaContextView){.world = &context->world, .program = &context->program,
        .control = &context->control, .status = context->status};
    return ama_status_success();
}

/** Move the linear C owner without an admission gap. This does not publish a
 * cross-thread handoff or end any native borrow. Executor transfer requires the
 * host's explicit synchronized ownership boundary and native-affinity checks.
 * Component contract: [PAR-01](../../../spec/language/async-and-parallelism.md#par-01), [EMBED-03](../../../spec/implementation/c-library.md#embed-03), [PAR-02](../../../spec/language/async-and-parallelism.md#par-02). */
AmaStatus ama_context_lease_move(AmaContextLease *destination, AmaContextLease *source) {
    if (destination == NULL || source == NULL || destination == source
        || destination->context != NULL || destination->registry != NULL || destination->identity != 0
        || ama_context_leased(source) == NULL) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    *destination = *source;
    *source = (AmaContextLease){0};
    return ama_status_success();
}
