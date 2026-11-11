#include "runtime/objects/internal.h"

static AmaStatus protect(AmaObjectOperation *operation, AmaHandle handle, AmaHandle parent, bool copy) {
    AmaObjectType *type = NULL;
    AmaPoolCell *cell = NULL;
    AmaStatus status = ama_object_lookup(operation->world, handle, false, &type, &cell);
    if (status.code != AMA_OK) { return status; }
    if (parent.generation != 0 && !ama_same_handle(cell->owner, parent)) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    status = ama_instance_access(&cell->metadata, AMA_ACCESS_READ);
    if (status.code != AMA_OK) { return status; }
    if (copy && type->hooks.initialize != NULL && type->hooks.clone == NULL) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    AmaObjectProtection *record = source_arena_allocate(&operation->arena, sizeof(*record));
    if (record == NULL) { return ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0); }
    *record = (AmaObjectProtection){.type = type, .cell = cell, .handle = handle,
        .previous = cell->metadata.service, .copy = copy, .restore = true};
    if (operation->protected_last == NULL) { operation->protected_first = record; }
    else { operation->protected_last->next = record; }
    operation->protected_last = record;
    cell->metadata.service.activity = AMA_INSTANCE_ACQUIRED;
    return ama_status_success();
}

AmaStatus ama_object_preflight(AmaObjectOperation *operation, AmaHandle root, bool copy, bool continuation) {
    (void)continuation;
    for (AmaObjectProtection *record = operation->protected_first; record != NULL; record = record->next) {
        if (record->copy && copy && ama_same_handle(record->handle, root)) {
            return ama_status_success();
        }
    }
    AmaObjectProtection *previous = operation->protected_last;
    AmaStatus status = protect(operation, root, (AmaHandle){0}, copy);
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
            status = protect(operation, child, record->handle, copy);
            if (status.code != AMA_OK) { return status; }
        }
    }
    return ama_status_success();
}

void ama_object_unprotect(AmaObjectOperation *operation) {
    for (AmaObjectProtection *record = operation->protected_first; record != NULL; record = record->next) {
        if (record->restore && record->cell->occupied
            && record->cell->metadata.identity.generation == record->handle.generation) {
            record->cell->metadata.service = record->previous;
        }
    }
}
