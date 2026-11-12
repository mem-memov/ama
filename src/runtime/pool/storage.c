/**
 * @file
 * @brief Validate aligned pool layout and access active or staged payload banks.
 *
 * [RT-02](../../../spec/implementation/runtime.md#rt-02)
 * All size arithmetic is checked before allocation. Public lookup checks exact identity before exposing storage.
 */
#include <stdint.h>
#include <string.h>
#include "internal.h"

/** Initialize checked two-bank layout using a nonzero nominal ID, power-of-two alignment, slot count and generation limit. Own no payload resource until reservation/construction. */
AmaStatus ama_pool_init_aligned(AmaPool *pool, SourceAllocator allocator, uint64_t type_id,
    size_t size, size_t alignment, size_t slots, uint64_t limit) {
    *pool = (AmaPool){.free_head = SIZE_MAX};
    if (alignment == 0 || (alignment & (alignment - 1)) != 0) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    if (alignment < _Alignof(max_align_t)) { alignment = _Alignof(max_align_t); }
    if (sizeof(AmaPoolCell) > SIZE_MAX - (alignment - 1)) {
        return ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0);
    }
    size_t offset = sizeof(AmaPoolCell) + (alignment - sizeof(AmaPoolCell) % alignment) % alignment;
    if (type_id == 0 || slots == 0 || limit == 0 || size > SIZE_MAX - (alignment - 1)) {
        return ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0);
    }
    size_t bank = (size + alignment - 1) / alignment * alignment;
    if (bank > (SIZE_MAX - offset) / 2) {
        return ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0);
    }
    size_t stride = offset + bank * 2;
    if (stride > SIZE_MAX / slots || stride * slots > SIZE_MAX - (alignment - 1)) {
        return ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0);
    }
    *pool = (AmaPool){.allocator = allocator, .type_id = type_id, .generation_limit = limit,
        .alignment = alignment, .payload_size = size, .payload_offset = offset, .payload_stride = bank, .stride = stride,
        .slots_per_block = slots, .free_head = SIZE_MAX};
    return ama_status_success();
}

/** Initialize the pool with max_align_t alignment; extended native alignment must use the explicit aligned operation. */
AmaStatus ama_pool_init(AmaPool *pool, SourceAllocator allocator, uint64_t type_id,
    size_t size, size_t slots, uint64_t limit) {
    return ama_pool_init_aligned(pool, allocator, type_id, size, _Alignof(max_align_t), slots, limit);
}

/** Free all remaining pool allocations and reset metadata. The caller must already have finalized every live payload; this operation invokes no destructors. */
void ama_pool_dispose(AmaPool *pool) {
    for (size_t index = 0; index < pool->block_count; index++) {
        pool->allocator.release(pool->allocator.context, pool->blocks[index].allocation);
    }
    pool->allocator.release(pool->allocator.context, pool->blocks);
    *pool = (AmaPool){.free_head = SIZE_MAX};
}

/** Borrow the cell at a bounded historical slot, or null for missing/reclaimed storage; no publication or generation validation is implied. */
AmaPoolCell *ama_pool_cell(const AmaPool *pool, size_t slot) {
    size_t block = slot / pool->slots_per_block;
    if (block >= pool->block_count || pool->blocks[block].memory == NULL) {
        return NULL;
    }
    return (AmaPoolCell *)(void *)(pool->blocks[block].memory + slot % pool->slots_per_block * pool->stride);
}

/** Borrow the active bank, or explicit private preparation payload during construction; valid cell/layout preconditions are supplied by the caller. */
void *ama_pool_payload(const AmaPool *pool, AmaPoolCell *cell) {
    return cell->preparation_payload != NULL ? cell->preparation_payload
        : (unsigned char *)(void *)cell + pool->payload_offset + (cell->alternate_payload ? pool->payload_stride : 0);
}

/** Borrow the inactive replacement bank without changing active storage. It stages lifecycle replacement, never ordinary execution state. */
void *ama_pool_spare_payload(const AmaPool *pool, AmaPoolCell *cell) {
    return (unsigned char *)(void *)cell + pool->payload_offset + (cell->alternate_payload ? 0 : pool->payload_stride);
}

/** Validate empty/type/bounds/occupancy/generation and, for public access, live identity before publishing the borrowed cell output. Reserved access is trusted private-phase authority. */
AmaStatus ama_pool_lookup(const AmaPool *pool, AmaHandle handle, bool reserved, AmaPoolCell **output) {
    if (ama_handle_is_empty(handle)) {
        return ama_status_make(AMA_EMPTY_REFERENCE, NULL, 0);
    }
    if (handle.type_id != pool->type_id) {
        return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0);
    }
    AmaPoolCell *cell = ama_pool_cell(pool, handle.slot_id);
    if (cell == NULL || !cell->occupied || cell->metadata.identity.generation != handle.generation
        || (!reserved && !cell->metadata.identity.live)) {
        return ama_status_make(AMA_STALE_REFERENCE, NULL, 0);
    }
    AmaStatus status = reserved ? ama_status_success()
        : ama_validate_handle(handle, pool->type_id, &cell->metadata.identity);
    if (status.code == AMA_OK) { *output = cell; }
    return status;
}
