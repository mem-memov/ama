#include "runtime/objects/internal.h"

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
