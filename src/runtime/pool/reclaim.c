/**
 * @file
 * @brief Release every entirely free payload block without renumbering surviving identities.
 *
 * [RT-13](../../../spec/implementation/runtime.md#rt-13) requires sparse directory
 * holes, including before live later blocks. Free links exclude every reclaimable
 * block before storage is released. [REF-06](../../../spec/language/values-and-references.md#ref-06)
 * is preserved by permanently retiring absent historical entries; growth appends
 * new identifiers rather than rematerializing retired slots.
 */
#include <stdint.h>
#include "internal.h"

/** Rebuild the exact free set of retained occupied blocks; absent and entirely free blocks cannot contribute any link. */
static void ama_pool_rebuild_free(AmaPool *pool) {
    pool->free_head = SIZE_MAX;
    for (size_t block = pool->block_count; block != 0;) {
        block--;
        if (pool->blocks[block].memory == NULL || pool->blocks[block].occupied == 0) {
            continue;
        }
        for (size_t index = pool->slots_per_block; index != 0;) {
            size_t slot = block * pool->slots_per_block + --index;
            AmaPoolCell *cell = ama_pool_cell(pool, slot);
            if (!cell->occupied && !cell->metadata.identity.retired) {
                cell->next_free = pool->free_head;
                pool->free_head = slot;
            }
        }
    }
}

/** Release all unoccupied blocks after unlinking their slots. Historical holes remain absent forever; occupied blocks and their addresses are unchanged. */
AmaStatus ama_pool_reclaim(AmaPool *pool, size_t *released) {
    ama_pool_rebuild_free(pool);
    *released = 0;
    for (size_t block = 0; block < pool->block_count; block++) {
        AmaPoolBlock *entry = &pool->blocks[block];
        if (entry->memory != NULL && entry->occupied == 0) {
            void *allocation = entry->allocation;
            entry->memory = NULL;
            entry->allocation = NULL;
            pool->allocated_blocks--;
            (*released)++;
            pool->allocator.release(pool->allocator.context, allocation);
        }
    }
    return ama_status_success();
}
