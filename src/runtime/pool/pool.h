/**
 * @file
 * @brief Own stable slot blocks, exact generation history and private replacement banks.
 *
 * [RT-02](../../../spec/implementation/runtime.md#rt-02)
 * Callers finalize resources before disposal. Directory history is retained so reclaimed slot IDs cannot revive.
 * [LIFE-05](../../../spec/implementation/native-interface.md#life-05) root-order
 * links belong to stable cell metadata and survive payload-bank replacement.
 */
#ifndef AMA_RUNTIME_POOL_H
#define AMA_RUNTIME_POOL_H

#include <stddef.h>
#include "runtime/metadata/instance.h"
#include "source/memory/allocator.h"

/** Stable cell metadata and owner/free/traversal links. root_previous/root_next and
 * nonzero root_type_id register only successful independent roots for
 * [LIFE-05](../../../spec/implementation/native-interface.md#life-05) cleanup.
 * These runtime links survive payload-bank replacement and are cleared before
 * slot reuse. preparation_payload redirects private construction without moving resources.
 */
typedef struct AmaPoolCell {
    AmaInstanceMetadata metadata;
    AmaHandle owner;
    struct AmaPoolCell *root_previous;
    struct AmaPoolCell *root_next;
    uint64_t root_type_id;
    size_t next_free;
    size_t traversal_child;
    bool occupied;
    bool alternate_payload;
    void *preparation_payload;
} AmaPoolCell;

/** One aligned payload block and its original allocator base; occupied counts private and published reservations. */
typedef struct {
    unsigned char *memory;
    void *allocation;
    size_t occupied;
} AmaPoolBlock;

/** Pool-owned block directory and aligned two-bank slot layout. block_count retains historical identity extent; allocated_blocks tracks physical blocks separately. */
typedef struct {
    SourceAllocator allocator;
    uint64_t type_id;
    uint64_t generation_limit;
    size_t slots_per_block;
    size_t payload_size;
    size_t alignment;
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

/* One pool per executable type. Alignment is explicit; init defaults to max_align_t. Reserved slots
 * are private construction storage; only publish makes their handles accessible.
 * Lookup borrows stable storage until release/disposal, never beyond its lifetime. */
/** Initialize the pool with max_align_t alignment; extended native alignment must use the explicit aligned operation. */
AmaStatus ama_pool_init(AmaPool *, SourceAllocator, uint64_t type_id,
    size_t payload_size, size_t slots_per_block, uint64_t generation_limit);
/** Initialize checked two-bank layout using a nonzero nominal ID, power-of-two alignment, slot count and generation limit. Own no payload resource until reservation/construction. */
AmaStatus ama_pool_init_aligned(AmaPool *, SourceAllocator, uint64_t type_id,
    size_t payload_size, size_t alignment, size_t slots_per_block, uint64_t generation_limit);
/** Free all remaining pool allocations and reset metadata. The caller must already have finalized every live payload; this operation invokes no destructors. */
void ama_pool_dispose(AmaPool *);
/** Pop or grow a free slot, preserve its identity generation and reset private metadata/banks. Publish handle/cell outputs only on success; no native hook runs. */
AmaStatus ama_pool_reserve(AmaPool *, AmaHandle *, AmaPoolCell **);
/** Validate empty/type/bounds/occupancy/generation and, for public access, live identity before publishing the borrowed cell output. Reserved access is trusted private-phase authority. */
AmaStatus ama_pool_lookup(const AmaPool *, AmaHandle, bool reserved, AmaPoolCell **);
/** Borrow the active bank, or explicit private preparation payload during construction; valid cell/layout preconditions are supplied by the caller. */
void *ama_pool_payload(const AmaPool *, AmaPoolCell *);
/** Borrow the inactive replacement bank without changing active storage. It stages lifecycle replacement, never ordinary execution state. */
void *ama_pool_spare_payload(const AmaPool *, AmaPoolCell *);
/** Require an occupied private slot with complete Live lifecycle. Set public identity live and service Ready; increment live count once. */
AmaStatus ama_pool_publish(AmaPool *, AmaHandle);
/** Resolve a public handle, clear its identity live bit and decrement live count before resource cleanup. */
AmaStatus ama_pool_invalidate(AmaPool *, AmaHandle);
/** Require invalidated, inactive Fresh/Finalized storage. Advance generation before reuse or permanently retire at exhaustion; reject live or active-cleanup release. */
AmaStatus ama_pool_release(AmaPool *, AmaHandle);
/** Synchronous published-slot visitor; false ends enumeration without mutating the pool. */
typedef bool (*AmaPoolVisitor)(void *, AmaHandle, const AmaPoolCell *);
/** Stream current live published identities under the caller's exclusive pool/context authority.
 * No native payload, constructor or acquisition is touched; holes and private reservations are skipped.
 */
AmaStatus ama_pool_visit(const AmaPool *, AmaPoolVisitor, void *);
/** Free every entirely unoccupied block, including before live later blocks, and retain null directory history forever. Report released block count; live payload addresses and all old identity exclusions remain unchanged. */
AmaStatus ama_pool_reclaim(AmaPool *, size_t *released);

#endif
