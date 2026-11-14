/**
 * @file
 * @brief Run checked membership edits with one scoped lifecycle operation.
 *
 * [LIST-04](../../../../spec/language/lists.md#list-04) forbids action selectors
 * and retained results. Direct C entry points return their status to the caller.
 * [LIST-10](../../../../spec/language/lists.md#list-10) requires preflight before
 * destructive effects. Typed method wrappers provide exact service authority;
 * [LIST-13](../../../../spec/language/lists.md#list-13) preflights the fixed
 * lexical catalog before native work, then corrects every matching boundary.
 */
#include "internal.h"

/** Classify only callback-free identity/availability preflight's exact core
 * codomain. Native constructor/destructor results never enter this predicate.
 */
static bool ama_list_preflight_unavailable(AmaStatus status) {
    return status.code == AMA_BUSY || status.code == AMA_STALE_REFERENCE || status.code == AMA_EMPTY_REFERENCE;
}

/** Borrow owner storage and establish the world's nonreentrant operation guard.
 * Failure leaves no arena/guard to release. Success requires exactly one finish.
 */
static AmaStatus ama_list_storage_begin(AmaListStorage *storage, AmaListMutation *mutation) {
    AmaWorld *world = storage->world;
    if (world->operation_active || world->failed) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    *mutation = (AmaListMutation){.objects = {.world = world}, .owner = storage->owner,
        .list = storage->list, .type = storage->element_type, .owning = storage->owning};
    AmaObjectType *parent_type = NULL;
    AmaStatus status = ama_object_lookup(world, storage->owner, false, &parent_type, &mutation->parent);
    if (status.code != AMA_OK) {
        return status;
    }
    status = ama_list_cursors_prepare(parent_type, mutation->parent, storage->owner,
        storage->list, &mutation->cursors);
    if (status.code != AMA_OK) { return status; }
    source_arena_init(&mutation->objects.arena, world->allocator);
    world->operation_active = true;
    return ama_status_success();
}

/** Release all preflight protection/scratch after success or partial failure. */
static void ama_list_storage_finish(AmaListMutation *mutation) {
    ama_object_unprotect(&mutation->objects);
    source_arena_destroy(&mutation->objects.arena);
    mutation->objects.world->operation_active = false;
}

AmaStatus ama_list_storage_insert_checked(AmaListStorage *storage, size_t position,
    AmaHandle source, bool require_copy, bool *refused) {
    *refused = false;
    if (position > storage->list->count) {
        *refused = true;
        return ama_status_make(AMA_INDEX_BOUNDS, NULL, 0);
    }
    AmaListMutation mutation;
    AmaStatus status = ama_list_storage_begin(storage, &mutation);
    if (status.code != AMA_OK) {
        return status;
    }
    if (storage->owning && require_copy) {
        status = ama_object_copy_capability(&mutation.objects, storage->element_type);
    }
    if (status.code == AMA_OK && storage->owning && source.generation != 0) {
        status = ama_object_preflight(&mutation.objects, source, true, false);
        /* This callback-free checker produces these codes only at exact identity
         * or availability checks. Later allocator/native stages never set this
         * flag, so equal numeric native failures cannot become refusals. */
        *refused = ama_list_preflight_unavailable(status);
    }
    if (status.code == AMA_OK) {
        status = ama_list_insert(&mutation, position, source);
    }
    ama_list_storage_finish(&mutation);
    return status;
}

AmaStatus ama_list_storage_insert(AmaListStorage *storage, size_t position, AmaHandle source) {
    bool refused;
    return ama_list_storage_insert_checked(storage, position, source, false, &refused);
}

AmaStatus ama_list_storage_remove_checked(AmaListStorage *storage, size_t position, size_t count, bool *refused) {
    *refused = false;
    if (position > storage->list->count || count > storage->list->count - position) {
        *refused = true;
        return ama_status_make(AMA_INDEX_BOUNDS, NULL, 0);
    }
    AmaListMutation mutation;
    AmaStatus status = ama_list_storage_begin(storage, &mutation);
    if (status.code != AMA_OK) {
        return status;
    }
    if (storage->owning) {
        for (size_t index = 0; index < count; index++) {
            status = ama_object_preflight(&mutation.objects, *ama_handle_list_element(storage->list, position + index), false, false);
            if (status.code != AMA_OK) {
                *refused = ama_list_preflight_unavailable(status);
                break;
            }
        }
    }
    if (status.code == AMA_OK) {
        status = ama_list_remove(&mutation, position, count);
    }
    ama_list_storage_finish(&mutation);
    return status;
}

AmaStatus ama_list_storage_remove(AmaListStorage *storage, size_t position, size_t count) {
    bool refused;
    return ama_list_storage_remove_checked(storage, position, count, &refused);
}
