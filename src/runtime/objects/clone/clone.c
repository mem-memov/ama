#include "runtime/objects/internal.h"

AmaStatus ama_object_clone(AmaWorld *world, AmaHandle source, bool continuation, AmaHandle *output) {
    if (world->operation_active || world->failed) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    world->operation_active = true;
    AmaObjectOperation operation = {.world = world};
    source_arena_init(&operation.arena, world->allocator);
    AmaStatus status = ama_object_preflight(&operation, source, true, continuation);
    AmaHandle result = {0};
    if (status.code == AMA_OK) {
        status = ama_object_clone_prepare(&operation, source, (AmaHandle){0}, continuation, NULL, &result);
    }
    if (status.code == AMA_OK) {
        for (AmaObjectWork *work = operation.first; work != NULL; work = work->next) {
            AmaServiceState service = work->cell->metadata.service;
            (void)ama_pool_publish(&work->type->pool, work->handle);
            work->cell->metadata.service = service;
        }
        *output = result;
    }
    ama_object_unprotect(&operation);
    source_arena_destroy(&operation.arena);
    world->operation_active = false;
    return status;
}
