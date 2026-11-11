#include <stdint.h>
#include <string.h>
#include "internal.h"

AmaStatus ama_pool_init(AmaPool *pool, SourceAllocator allocator, uint64_t type_id,
    size_t size, size_t slots, uint64_t limit) {
    *pool = (AmaPool){.free_head = SIZE_MAX};
    size_t alignment = _Alignof(max_align_t);
    size_t offset = sizeof(AmaPoolCell) + (alignment - sizeof(AmaPoolCell) % alignment) % alignment;
    if (type_id == 0 || slots == 0 || limit == 0 || size > SIZE_MAX - offset
        || size + offset > SIZE_MAX - (alignment - 1)) {
        return ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0);
    }
    size_t stride = (offset + size + alignment - 1) / alignment * alignment;
    if (stride > SIZE_MAX / slots) {
        return ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0);
    }
    *pool = (AmaPool){.allocator = allocator, .type_id = type_id, .generation_limit = limit,
        .payload_size = size, .payload_offset = offset, .stride = stride,
        .slots_per_block = slots, .free_head = SIZE_MAX};
    return ama_status_success();
}

void ama_pool_dispose(AmaPool *pool) {
    for (size_t index = 0; index < pool->block_count; index++) {
        pool->allocator.release(pool->allocator.context, pool->blocks[index].memory);
    }
    pool->allocator.release(pool->allocator.context, pool->blocks);
    *pool = (AmaPool){.free_head = SIZE_MAX};
}

AmaPoolCell *ama_pool_cell(const AmaPool *pool, size_t slot) {
    size_t block = slot / pool->slots_per_block;
    if (block >= pool->block_count || pool->blocks[block].memory == NULL) {
        return NULL;
    }
    return (AmaPoolCell *)(void *)(pool->blocks[block].memory + slot % pool->slots_per_block * pool->stride);
}

void *ama_pool_payload(const AmaPool *pool, AmaPoolCell *cell) {
    return (unsigned char *)(void *)cell + pool->payload_offset;
}

AmaStatus ama_pool_lookup(const AmaPool *pool, AmaHandle handle, bool reserved, AmaPoolCell **output) {
    if (handle.generation == 0) {
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
    *output = cell;
    return ama_status_success();
}
