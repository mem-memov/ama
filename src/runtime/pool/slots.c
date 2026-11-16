/**
 * @file
 * @brief Reserve, publish, invalidate and retire individual slot identities.
 *
 * [RT-12](../../../spec/implementation/runtime.md#rt-12)
 * Private construction becomes visible only after a complete lifecycle ledger; release never wraps a generation.
 */
#include <stdint.h>
#include <string.h>
#include "internal.h"

/** Pop or grow a free slot, preserve its identity generation and reset private metadata/banks. Publish handle/cell outputs only on success; no native hook runs. Component contract: [RT-12](../../../spec/implementation/runtime.md#rt-12). */
AmaStatus ama_pool_reserve(AmaPool *pool, AmaHandle *handle, AmaPoolCell **output) {
    if (pool->free_head == SIZE_MAX) {
        AmaStatus status = ama_pool_grow(pool);
        if (!ama_status_is_success(status)) {
            return status;
        }
    }
    AmaPoolCell *cell = ama_pool_cell(pool, pool->free_head);
    pool->free_head = cell->next_free;
    AmaSlotIdentity identity = cell->metadata.identity;
    memset(cell, 0, pool->stride);
    cell->metadata.identity = identity;
    cell->occupied = true;
    pool->blocks[identity.slot_id / pool->slots_per_block].occupied++;
    *handle = (AmaHandle){pool->type_id, identity.slot_id, identity.generation};
    *output = cell;
    return ama_status_success();
}

/** Require an occupied private slot with complete Live lifecycle. Set public identity live and service Ready; increment live count once. Component contract: [RT-12](../../../spec/implementation/runtime.md#rt-12). */
AmaStatus ama_pool_publish(AmaPool *pool, AmaHandle handle) {
    AmaPoolCell *cell = NULL;
    AmaStatus status = ama_pool_lookup(pool, handle, true, &cell);
    if (!ama_status_is_success(status)) {
        return status;
    }
    if (cell->metadata.identity.live || cell->metadata.lifetime.phase != AMA_LIFETIME_LIVE) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    cell->metadata.identity.live = true;
    cell->metadata.service.activity = AMA_INSTANCE_READY;
    pool->live_count++;
    return ama_status_success();
}

/** Resolve a public handle, clear its identity live bit and decrement live count before resource cleanup. Component contract: [RT-12](../../../spec/implementation/runtime.md#rt-12). */
AmaStatus ama_pool_invalidate(AmaPool *pool, AmaHandle handle) {
    AmaPoolCell *cell = NULL;
    AmaStatus status = ama_pool_lookup(pool, handle, false, &cell);
    if (!ama_status_is_success(status)) {
        return status;
    }
    cell->metadata.identity.live = false;
    pool->live_count--;
    return ama_status_success();
}

/** Require invalidated, inactive Fresh/Finalized storage. Advance generation before reuse or permanently retire at exhaustion; reject live or active-cleanup release. Component contract: [RT-12](../../../spec/implementation/runtime.md#rt-12). */
AmaStatus ama_pool_release(AmaPool *pool, AmaHandle handle) {
    AmaPoolCell *cell = NULL;
    AmaStatus status = ama_pool_lookup(pool, handle, true, &cell);
    if (!ama_status_is_success(status)) {
        return status;
    }
    AmaLifetime *lifetime = &cell->metadata.lifetime;
    if (cell->metadata.identity.live || lifetime->cleanup_active
        || (lifetime->phase != AMA_LIFETIME_FRESH && lifetime->phase != AMA_LIFETIME_FINALIZED)) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    cell->occupied = false;
    pool->blocks[handle.slot_id / pool->slots_per_block].occupied--;
    status = ama_slot_advance_generation_limited(&cell->metadata.identity, pool->generation_limit);
    if (status.code == AMA_LIMIT_EXCEEDED && cell->metadata.identity.retired) {
        return ama_status_success();
    }
    if (status.code == AMA_OK) {
        cell->next_free = pool->free_head;
        pool->free_head = handle.slot_id;
    }
    return status;
}
