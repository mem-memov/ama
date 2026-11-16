/**
 * @file
 * @brief Commit a fully prepared replacement while preserving root identity and native addresses.
 *
 * [COPY-02](../../../../spec/language/values-and-references.md#copy-02)
 * The spare bank stages lifecycle replacement only; ordinary execution uses active storage directly and never saves/restores a working copy.
 */
#include <string.h>
#include "runtime/objects/internal.h"

/** After successful old-value cleanup, flip to the prepared payload bank and publish the new ledger/children under the unchanged root identity. Release only scratch root identity; native payloads are never moved. Component contract: [COPY-02](../../../../spec/language/values-and-references.md#copy-02). */
void ama_object_replacement_publish(AmaObjectOperation *operation, AmaObjectType *type, AmaPoolCell *cell, AmaHandle destination) {
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

/** Protect source/destination, prepare in the inactive destination bank, then destroy old state and publish. Preparation failure preserves old state; cleanup failure terminates the world and cleans prepared resources. Component contract: [COPY-02](../../../../spec/language/values-and-references.md#copy-02). */
static AmaStatus ama_replacement_commit(AmaObjectOperation *operation, AmaHandle destination, AmaHandle source, bool continuation) {
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
    if (status.code == AMA_OK) { ama_object_replacement_publish(operation, type, cell, destination); }
    else {
        operation->world->failed = true;
        status = ama_object_cleanup(operation->world, prepared, status);
    }
    return status;
}

/** Apply exact copy/overlap checks inside a caller-owned guarded operation; prepare/commit only when runtime identity is not an ordinary self-copy. Component contract: [COPY-02](../../../../spec/language/values-and-references.md#copy-02). */
AmaStatus ama_object_replace_in(AmaObjectOperation *operation, AmaHandle destination, AmaHandle source, bool continuation) {
    bool no_op = false;
    AmaStatus status = ama_object_copy_checks(operation->world, destination, source, continuation, &no_op);
    return status.code == AMA_OK && !no_op ? ama_replacement_commit(operation, destination, source, continuation) : status;
}

/** Own one nonreentrant world operation, its protection records and scratch for replacement; release all temporary authority on every return. Component contract: [COPY-02](../../../../spec/language/values-and-references.md#copy-02). */
AmaStatus ama_object_replace(AmaWorld *world, AmaHandle destination, AmaHandle source, bool continuation) {
    if (world->operation_active || world->failed) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    world->operation_active = true;
    AmaObjectOperation operation = {.world = world};
    source_arena_init(&operation.arena, world->allocator);
    AmaStatus status = ama_object_replace_in(&operation, destination, source, continuation);
    ama_object_unprotect(&operation);
    source_arena_destroy(&operation.arena);
    world->operation_active = false;
    return status;
}
