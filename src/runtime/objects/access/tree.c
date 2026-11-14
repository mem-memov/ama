/**
 * @file
 * @brief Protect complete owned trees before any clone or destructive native effect.
 *
 * [COPY-07](../../../../spec/language/values-and-references.md#copy-07)
 * References are not traversed. Epochs detect repeated ownership and borrowed source authority permits only authorized nested reads.
 */
#include <stdint.h>
#include "runtime/objects/internal.h"

/** Validate parent ownership, capability and availability before appending one protection record. Preserve prior service state; same-epoch duplicate ownership fails, earlier authorized copy protection may be reused. */
static AmaStatus ama_tree_protect(AmaObjectOperation *operation, AmaHandle handle, AmaHandle parent, bool copy) {
    AmaObjectType *type = NULL;
    AmaPoolCell *cell = NULL;
    AmaStatus status = ama_object_source_lookup(operation, handle, &type, &cell);
    if (status.code != AMA_OK) { return status; }
    if (parent.generation != 0 && !ama_same_handle(cell->owner, parent)) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    for (AmaObjectProtection *record = operation->protected_first; record != NULL; record = record->next) {
        if (copy && record->copy && ama_same_handle(record->handle, handle)) {
            if (record->epoch == operation->protection_epoch) {
                return ama_status_make(AMA_INVALID_STATE, NULL, 0);
            }
            record->epoch = operation->protection_epoch;
            return ama_status_success();
        }
    }
    const AmaObjectProtection *authority = NULL;
    if (copy) {
        for (const AmaObjectProtection *record = operation->source_authority; record != NULL; record = record->next) {
            if (record->copy && ama_same_handle(record->handle, handle)) { authority = record; break; }
        }
    }
    if (authority == NULL) {
        status = cell->metadata.identity.live ? ama_instance_access(&cell->metadata, AMA_ACCESS_READ)
            : ama_service_access(&cell->metadata.service, AMA_ACCESS_READ);
        if (status.code != AMA_OK) { return status; }
    }
    if (copy && type->hooks.initialize != NULL && type->hooks.clone == NULL) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    AmaObjectProtection *record = source_arena_allocate(&operation->arena, sizeof(*record));
    if (record == NULL) { return ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0); }
    *record = (AmaObjectProtection){.type = type, .cell = cell, .handle = handle,
        .previous = authority == NULL ? cell->metadata.service : authority->previous,
        .epoch = operation->protection_epoch, .copy = copy, .restore = authority == NULL};
    if (operation->protected_last == NULL) { operation->protected_first = record; }
    else { operation->protected_last->next = record; }
    operation->protected_last = record;
    cell->metadata.service.activity = AMA_INSTANCE_ACQUIRED;
    return ama_status_success();
}

/** Protect the complete initialized owned tree before hooks or destruction. Follow ownership only, check declared copy capability and nonwrapping epochs; caller unprotects partial records on failure. */
AmaStatus ama_object_preflight(AmaObjectOperation *operation, AmaHandle root, bool copy, bool continuation) {
    AmaObjectType *type;
    AmaPoolCell *cell;
    AmaStatus identity = ama_object_source_lookup(operation, root, &type, &cell);
    if (identity.code != AMA_OK) { return identity; }
    for (AmaObjectProtection *record = operation->protected_first; record != NULL; record = record->next) {
        if (record->copy && copy && ama_same_handle(record->handle, root)) {
            return continuation ? ama_object_control_preflight(operation, root) : ama_status_success();
        }
    }
    if (copy) {
        AmaStatus capability = ama_object_copy_capability(operation, root.type_id);
        if (capability.code != AMA_OK) { return capability; }
    }
    if (operation->protection_epoch == SIZE_MAX) { return ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0); }
    operation->protection_epoch++;
    AmaObjectProtection *previous = operation->protected_last;
    AmaStatus status = ama_tree_protect(operation, root, (AmaHandle){0}, copy);
    if (status.code != AMA_OK) { return status; }
    AmaObjectProtection *record = previous == NULL ? operation->protected_first : previous->next;
    for (; record != NULL; record = record->next) {
        AmaObjectWork work = {.type = record->type, .cell = record->cell};
        size_t count = record->cell->metadata.lifetime.live_children;
        for (size_t index = 0; index < count; index++) {
            uint64_t type_id;
            const AmaRecipe *recipe;
            AmaHandle child = *ama_object_child(&work, index, &type_id, &recipe);
            if (child.type_id != type_id) { return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0); }
            status = ama_tree_protect(operation, child, record->handle, copy);
            if (status.code != AMA_OK) { return status; }
        }
    }
    return copy && continuation ? ama_object_control_preflight(operation, root) : ama_status_success();
}

/** Restore only records that own restoration authority and still identify the same occupied generation; never resurrect released or replaced identities. */
void ama_object_unprotect(AmaObjectOperation *operation) {
    for (AmaObjectProtection *record = operation->protected_first; record != NULL; record = record->next) {
        if (record->restore && record->cell->occupied
            && record->cell->metadata.identity.generation == record->handle.generation) {
            record->cell->metadata.service = record->previous;
        }
    }
}
