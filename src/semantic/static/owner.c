/**
 * @file
 * @brief Own stable static inventory records and release partial construction deterministically.
 *
 * [ARCH-03](../../../spec/implementation/compiler.md#arch-03) requires explicit
 * arena ownership; borrowed loaded source definitions always outlive this owner.
 */
#include <assert.h>
#include <string.h>
#include "internal.h"

/** Initialize without allocation or mutation of the valid borrowed project.
 * Contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03).
 */
void static_inventory_init(StaticInventory *inventory, const Project *project, SourceAllocator allocator) {
    assert(project != NULL && project->valid);
    *inventory = (StaticInventory){.project = project};
    source_arena_init(&inventory->arena, allocator);
}

/** Destroy records through the arena, not recursive signature links; zero all
 * publication/borrowed pointers. Contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03).
 */
void static_inventory_destroy(StaticInventory *inventory) {
    source_arena_destroy(&inventory->arena);
    *inventory = (StaticInventory){0};
}

/** Zero successful scratch and stop allocating after the first failure.
 * Contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03)
 * leaves both successful and refused construction under one teardown boundary.
 */
void *static_allocate(StaticInventory *inventory, size_t size) {
    if (inventory->status != STATIC_OK) { return NULL; }
    void *record = source_arena_allocate(&inventory->arena, size);
    if (record == NULL) { (void)static_memory_failure(inventory); }
    else { memset(record, 0, size); }
    return record;
}
