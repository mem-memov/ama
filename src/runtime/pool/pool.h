#ifndef AMA_RUNTIME_POOL_H
#define AMA_RUNTIME_POOL_H

#include <stddef.h>
#include "runtime/metadata/instance.h"
#include "source/memory/allocator.h"

typedef struct {
    AmaInstanceMetadata metadata;
    AmaHandle owner;
    size_t next_free;
    bool occupied;
    bool alternate_payload;
    void *preparation_payload;
} AmaPoolCell;

typedef struct {
    unsigned char *memory;
    size_t occupied;
} AmaPoolBlock;

typedef struct {
    SourceAllocator allocator;
    uint64_t type_id;
    uint64_t generation_limit;
    size_t slots_per_block;
    size_t payload_size;
    size_t payload_offset;
    size_t payload_stride;
    size_t stride;
    AmaPoolBlock *blocks;
    size_t block_count;
    size_t directory_capacity;
    size_t allocated_blocks;
    size_t live_count;
    size_t free_head;
} AmaPool;

/* One pool per executable type. Payload alignment is max_align_t. Reserved slots
 * are private construction storage; only publish makes their handles accessible.
 * Lookup borrows stable storage until release/disposal, never beyond its lifetime. */
AmaStatus ama_pool_init(AmaPool *, SourceAllocator, uint64_t type_id,
    size_t payload_size, size_t slots_per_block, uint64_t generation_limit);
void ama_pool_dispose(AmaPool *);
AmaStatus ama_pool_reserve(AmaPool *, AmaHandle *, AmaPoolCell **);
AmaStatus ama_pool_lookup(const AmaPool *, AmaHandle, bool reserved, AmaPoolCell **);
void *ama_pool_payload(const AmaPool *, AmaPoolCell *);
void *ama_pool_spare_payload(const AmaPool *, AmaPoolCell *);
AmaStatus ama_pool_publish(AmaPool *, AmaHandle);
AmaStatus ama_pool_invalidate(AmaPool *, AmaHandle);
AmaStatus ama_pool_release(AmaPool *, AmaHandle);
AmaStatus ama_pool_reclaim(AmaPool *, size_t *released);

#endif
