/**
 * @file
 * @brief Compose protected normal destruction and terminal-world cleanup.
 *
 * [RT-10](../../../../spec/implementation/runtime.md#rt-10)
 * One operation guard owns scratch and protections; terminal cleanup clears active links rather than treating them as a recoverable busy refusal.
 */
#include "runtime/objects/internal.h"

/** Acquire one world operation guard, preflight normal destruction or clear terminal active links, then clean the tree. Always release protections and scratch, preserving the primary failure. */
AmaStatus ama_object_destroy(AmaWorld *world, AmaHandle root) {
    if (world->operation_active) { return ama_status_make(AMA_BUSY, NULL, 0); }
    world->operation_active = true;
    AmaObjectOperation operation = {.world = world};
    source_arena_init(&operation.arena, world->allocator);
    AmaStatus status = world->failed ? ama_object_end_active_links(world, root)
        : ama_object_preflight(&operation, root, false, false);
    if (status.code == AMA_OK) { status = ama_object_cleanup(world, root, ama_status_success()); }
    ama_object_unprotect(&operation);
    source_arena_destroy(&operation.arena);
    world->operation_active = false;
    return status;
}
