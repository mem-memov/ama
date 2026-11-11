#include "runtime/objects/internal.h"

static AmaStatus ama_clone_reserve(AmaObjectOperation *operation, AmaHandle source,
    AmaObjectWork *parent, void *root_payload, AmaObjectWork **output) {
    AmaObjectType *type = ama_world_type(operation->world, source.type_id);
    AmaObjectWork *work = source_arena_allocate(&operation->arena, sizeof(*work));
    if (work == NULL) { return ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0); }
    *work = (AmaObjectWork){.type = type, .parent = parent, .source = source};
    work->clone_root = parent == NULL ? work : parent->clone_root;
    AmaStatus status = ama_pool_reserve(&type->pool, &work->handle, &work->cell);
    if (status.code != AMA_OK) { return status; }
    if (parent != NULL) { work->cell->owner = parent->handle; }
    if (root_payload != NULL) { work->cell->preparation_payload = root_payload; }
    if (operation->last == NULL) { operation->first = work; }
    else { operation->last->next = work; }
    operation->last = work;
    *output = work;
    return ama_object_clone_fields(operation->world, work);
}

AmaStatus ama_object_clone_payload(AmaObjectOperation *operation, AmaObjectWork *work, bool continuation) {
    AmaPoolCell *source = NULL;
    AmaStatus status = ama_pool_lookup(&work->type->pool, work->source, true, &source);
    if (status.code != AMA_OK) { return status; }
    if (work->type->hooks.clone != NULL) {
        void *destination = ama_pool_payload(&work->type->pool, work->cell);
        void *original = ama_pool_payload(&work->type->pool, source);
        status = work->type->hooks.clone((unsigned char *)destination + work->type->native_offset,
            (unsigned char *)original + work->type->native_offset);
        if (status.code != AMA_OK) { return status; }
    }
    status = ama_lifetime_finish(&work->cell->metadata.lifetime, work->type->hooks.initialize != NULL);
    for (AmaObjectProtection *record = operation->protected_first; record != NULL; record = record->next) {
        if (ama_same_handle(record->handle, work->source)) {
            return ama_service_clone(&record->previous, continuation, &work->cell->metadata.service);
        }
    }
    return status.code == AMA_OK ? ama_status_make(AMA_INVALID_STATE, NULL, 0) : status;
}

static AmaHandle ama_clone_mapped(const AmaObjectOperation *operation, const AmaObjectWork *boundary,
    AmaHandle source, AmaHandle final_root) {
    if (source.generation == 0) { return source; }
    for (const AmaObjectWork *work = operation->first; work != NULL; work = work->next) {
        if (work->clone_root == boundary && ama_same_handle(work->source, source)) {
            return work == boundary && final_root.generation != 0 ? final_root : work->handle;
        }
    }
    return source;
}

void ama_object_fix_references(AmaObjectOperation *operation, AmaObjectWork *boundary, AmaHandle final_root) {
    for (AmaObjectWork *work = operation->first; work != NULL; work = work->next) {
        if (work->clone_root != boundary) { continue; }
        void *storage = ama_pool_payload(&work->type->pool, work->cell);
        for (size_t field = 0; field < work->type->field_count; field++) {
            if (ama_field_owns(work->type->fields[field].kind)) { continue; }
            size_t count;
            AmaHandle *handles = ama_object_handles(work->type, storage, field, &count);
            for (size_t index = 0; index < count; index++) {
                handles[index] = ama_clone_mapped(operation, boundary, handles[index], final_root);
            }
        }
    }
}

AmaStatus ama_object_clone_prepare(AmaObjectOperation *operation, AmaHandle source, AmaHandle final_root,
    bool continuation, void *root_payload, AmaHandle *output) {
    AmaObjectWork *work = NULL;
    AmaStatus status = ama_clone_reserve(operation, source, NULL, root_payload, &work);
    AmaObjectWork *root = work;
    while (status.code == AMA_OK) {
        if (work->next_child < work->cell->metadata.lifetime.child_capacity) {
            uint64_t type_id;
            const AmaRecipe *recipe;
            AmaHandle *destination = ama_object_child(work, work->next_child, &type_id, &recipe);
            AmaPoolCell *original = NULL;
            (void)ama_pool_lookup(&work->type->pool, work->source, true, &original);
            AmaObjectWork source_work = {.type = work->type, .cell = original};
            AmaHandle child = *ama_object_child(&source_work, work->next_child, &type_id, &recipe);
            AmaObjectWork *child_work = NULL;
            status = ama_clone_reserve(operation, child, work, NULL, &child_work);
            if (child_work != NULL) {
                *destination = child_work->handle;
                work = child_work;
            }
        } else {
            status = ama_object_clone_payload(operation, work, continuation);
            if (status.code != AMA_OK || work->parent == NULL) { break; }
            work = work->parent;
            work->next_child++;
            status = ama_lifetime_child_ready(&work->cell->metadata.lifetime);
        }
    }
    if (status.code != AMA_OK) {
        ama_object_report(operation->world, status);
        while (work != NULL) {
            status = ama_object_cleanup(operation->world, work->handle, status);
            work = work->parent;
        }
        return status;
    }
    ama_object_fix_references(operation, root, final_root);
    *output = root->handle;
    return ama_status_success();
}
