#include <stdint.h>
#include "internal.h"

static void rebuild(AmaPool *pool, size_t retained) {
    pool->free_head = SIZE_MAX;
    for (size_t block = retained; block != 0;) {
        block--;
        if (pool->blocks[block].memory == NULL) {
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

AmaStatus ama_pool_reclaim(AmaPool *pool, size_t *released) {
    size_t retained = pool->block_count;
    while (retained != 0 && pool->blocks[retained - 1].occupied == 0) {
        retained--;
    }
    /* Remove links to every released slot before freeing its storage. Historical
     * directory entries remain null forever; future growth appends fresh IDs. */
    rebuild(pool, retained);
    *released = 0;
    for (size_t block = retained; block < pool->block_count; block++) {
        if (pool->blocks[block].memory != NULL) {
            pool->allocator.release(pool->allocator.context, pool->blocks[block].memory);
            pool->blocks[block].memory = NULL;
            pool->allocated_blocks--;
            (*released)++;
        }
    }
    return ama_status_success();
}
