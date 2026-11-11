#include <string.h>
#include "runtime/objects/internal.h"

static void publish(AmaObjectOperation *operation, AmaObjectType *type, AmaPoolCell *cell, AmaHandle destination) {
    AmaObjectWork *root = operation->first;
    cell->alternate_payload = !cell->alternate_payload;
    cell->metadata.lifetime = root->cell->metadata.lifetime;
    AmaServiceState service = root->cell->metadata.service;
    (void)ama_pool_publish(&type->pool, destination);
    cell->metadata.service = service;
    for (AmaObjectWork *work = root->next; work != NULL; work = work->next) {
        if (work->parent == root) { work->cell->owner = destination; }
        service = work->cell->metadata.service;
        (void)ama_pool_publish(&work->type->pool, work->handle);
        work->cell->metadata.service = service;
    }
    root->cell->preparation_payload = NULL;
    root->cell->metadata.lifetime = (AmaLifetime){.phase = AMA_LIFETIME_FINALIZED};
    (void)ama_pool_release(&type->pool, root->handle);
}

static AmaStatus replace(AmaObjectOperation *operation, AmaHandle destination, AmaHandle source, bool continuation) {
    AmaObjectType *type = NULL;
    AmaPoolCell *cell = NULL;
    AmaStatus status = ama_object_lookup(operation->world, destination, false, &type, &cell);
    if (status.code != AMA_OK) { return status; }
    status = ama_object_preflight(operation, source, true, continuation);
    if (status.code != AMA_OK) { return status; }
    status = ama_object_preflight(operation, destination, false, false);
    if (status.code != AMA_OK) { return status; }
    void *spare = ama_pool_spare_payload(&type->pool, cell);
    memset(spare, 0, type->pool.payload_size);
    AmaHandle prepared = {0};
    status = ama_object_clone_prepare(operation, source, destination, continuation, spare, &prepared);
    if (status.code != AMA_OK) { return status; }
    for (AmaObjectProtection *record = operation->protected_first; record != NULL; record = record->next) {
        if (ama_same_handle(record->handle, destination)) { record->restore = false; }
    }
    status = ama_object_cleanup_keep(operation->world, destination, ama_status_success(), true);
    if (status.code == AMA_OK) { publish(operation, type, cell, destination); }
    else {
        operation->world->failed = true;
        status = ama_object_cleanup(operation->world, prepared, status);
    }
    return status;
}

AmaStatus ama_object_replace(AmaWorld *world, AmaHandle destination, AmaHandle source, bool continuation) {
    if (world->operation_active || world->failed) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    world->operation_active = true;
    AmaObjectOperation operation = {.world = world};
    source_arena_init(&operation.arena, world->allocator);
    bool no_op = false;
    AmaStatus status = ama_object_copy_checks(world, destination, source, continuation, &no_op);
    if (status.code == AMA_OK && !no_op) { status = replace(&operation, destination, source, continuation); }
    ama_object_unprotect(&operation);
    source_arena_destroy(&operation.arena);
    world->operation_active = false;
    return status;
}
