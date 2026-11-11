#include <stdint.h>
#include <string.h>
#include "internal.h"

static AmaStatus directory(AmaPool *pool) {
    if (pool->block_count < pool->directory_capacity) {
        return ama_status_success();
    }
    size_t capacity = pool->directory_capacity == 0 ? 1 : pool->directory_capacity;
    if (capacity > SIZE_MAX / 2 / sizeof(AmaPoolBlock)) {
        return ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0);
    }
    capacity *= 2;
    AmaPoolBlock *blocks = pool->allocator.allocate(pool->allocator.context, capacity * sizeof(*blocks));
    if (blocks == NULL) {
        return ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0);
    }
    if (pool->block_count != 0) {
        memcpy(blocks, pool->blocks, pool->block_count * sizeof(*blocks));
    }
    pool->allocator.release(pool->allocator.context, pool->blocks);
    pool->blocks = blocks;
    pool->directory_capacity = capacity;
    return ama_status_success();
}

AmaStatus ama_pool_grow(AmaPool *pool) {
    if (pool->block_count >= SIZE_MAX / pool->slots_per_block) {
        return ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0);
    }
    AmaStatus status = directory(pool);
    if (!ama_status_is_success(status)) {
        return status;
    }
    unsigned char *allocation = pool->allocator.allocate(pool->allocator.context,
        pool->slots_per_block * pool->stride + pool->alignment - 1);
    if (allocation == NULL) {
        return ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0);
    }
    size_t padding = (pool->alignment - (uintptr_t)allocation % pool->alignment) % pool->alignment;
    unsigned char *memory = allocation + padding;
    memset(memory, 0, pool->slots_per_block * pool->stride);
    size_t block = pool->block_count++;
    pool->blocks[block] = (AmaPoolBlock){.memory = memory, .allocation = allocation};
    pool->allocated_blocks++;
    for (size_t index = pool->slots_per_block; index != 0;) {
        size_t slot = block * pool->slots_per_block + --index;
        AmaPoolCell *cell = ama_pool_cell(pool, slot);
        cell->metadata.identity = (AmaSlotIdentity){pool->type_id, slot, 1, false, false};
        cell->next_free = pool->free_head;
        pool->free_head = slot;
    }
    return ama_status_success();
}
