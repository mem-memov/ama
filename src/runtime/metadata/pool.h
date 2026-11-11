#ifndef AMA_RUNTIME_METADATA_POOL_H
#define AMA_RUNTIME_METADATA_POOL_H

#include "identity.h"

/* Directory contents remain module-typed; these counters do not own payloads.
 * Generation history is separate from reclaimable payload blocks.
 */
typedef struct {
    uint64_t type_id;
    size_t slots_per_block;
    size_t allocated_blocks;
    size_t live_instances;
    bool has_free;
    size_t free_head;
} AmaPoolBookkeeping;

typedef struct {
    bool has_next;
    size_t next_slot;
} AmaFreeSlot;

#endif
