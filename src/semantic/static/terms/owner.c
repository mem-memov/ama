/**
 * @file
 * @brief Own immutable static term scratch independently of borrowed source inventories.
 *
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) keeps every
 * refused/partial construction destructible without recursive graph cleanup.
 */
#include <assert.h>
#include <string.h>
#include "internal.h"

/** Initialization performs no allocation, validation or source mutation.
 * Contract: [ARCH-03](../../../../spec/implementation/compiler.md#arch-03).
 */
void static_term_store_init(StaticTermStore *store, const StaticInventory *inventory, SourceAllocator allocator) {
    assert(inventory != NULL && inventory->valid);
    *store = (StaticTermStore){.inventory = inventory};
    source_arena_init(&store->arena, allocator);
}

/** Free arena records before expiring borrowed metadata; value/definition edges
 * do not participate in destruction or change any source lifecycle state.
 * Contract: [ARCH-03](../../../../spec/implementation/compiler.md#arch-03).
 */
void static_term_store_destroy(StaticTermStore *store) {
    source_arena_destroy(&store->arena);
    *store = (StaticTermStore){0};
}

/** Zero each complete allocation before callers assign identity/link fields.
 * Contract: [ARCH-03](../../../../spec/implementation/compiler.md#arch-03).
 */
void *static_term_allocate(StaticTermStore *store, size_t size) {
    if (store->status != STATIC_TERM_OK) { return NULL; }
    void *result = source_arena_allocate(&store->arena, size);
    if (result == NULL) { (void)static_term_memory_failure(store); }
    else { memset(result, 0, size); }
    return result;
}
