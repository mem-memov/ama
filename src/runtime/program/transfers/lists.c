#include <stdint.h>
#include <string.h>
#include "internal.h"

static size_t ama_transfer_list_count(const AmaProgramValue *value) {
    return value->shared_list != NULL ? value->shared_list->count : value->list->count;
}

static AmaStatus ama_transfer_list_preflight(AmaObjectOperation *operation, const AmaHandleList *list, bool copy) {
    for (size_t index = 0; index < list->count; index++) {
        AmaStatus status = ama_object_preflight(operation, list->items[index], copy, false);
        if (status.code != AMA_OK) { return status; }
    }
    return ama_status_success();
}

static AmaStatus ama_transfer_list_prepare(AmaProgram *program, AmaProgramValue *destination,
    const AmaProgramValue *source, const AmaFieldRecipe *initializers, AmaObjectOperation *operation, AmaHandleList *prepared) {
    size_t count = source != NULL ? ama_transfer_list_count(source) : initializers == NULL ? 0 : initializers->count;
    if (count > SIZE_MAX / sizeof(AmaHandle)) { return ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0); }
    if (count != 0) {
        prepared->items = program->world->allocator.allocate(program->world->allocator.context, count * sizeof(AmaHandle));
        if (prepared->items == NULL) { return ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0); }
        memset(prepared->items, 0, count * sizeof(AmaHandle));
    }
    for (size_t index = 0; index < count; index++) {
        AmaHandle handle = {0};
        if (destination->form == AMA_FIELD_REFERENCE_LIST) {
            handle = source != NULL ? (source->shared_list != NULL ? (AmaHandle){0} : source->list->items[index])
                : initializers != NULL && initializers->references != NULL ? initializers->references[index] : (AmaHandle){0};
        } else {
            AmaRecipe recipe = {0};
            AmaStatus status = ama_status_success();
            if (source != NULL) {
                AmaProgramValue element = {.type_id = source->type_id, .form = AMA_FIELD_OWNED};
                if (source->shared_list != NULL) { element.constant = &source->shared_list->items[index]; }
                else { element.handle = source->list->items[index]; }
                status = ama_program_value_recipe(program, &operation->arena, &element, source->continuation, &recipe);
            } else if (initializers != NULL && initializers->children != NULL) { recipe = initializers->children[index]; }
            if (status.code == AMA_OK) { status = ama_object_build(operation, destination->type_id, &recipe, &handle); }
            if (status.code != AMA_OK) { return status; }
        }
        prepared->items[prepared->count++] = handle;
    }
    return ama_status_success();
}

static AmaStatus ama_transfer_list_discard(AmaProgram *program, AmaHandleList *list, bool owning, AmaStatus primary) {
    while (list->count != 0) {
        AmaHandle handle = list->items[--list->count];
        if (owning) { primary = ama_object_cleanup(program->world, handle, primary); }
    }
    program->world->allocator.release(program->world->allocator.context, list->items);
    *list = (AmaHandleList){0};
    return primary;
}

AmaStatus ama_program_store_list(AmaProgram *program, AmaProgramValue *destination, const AmaProgramValue *source,
    const AmaFieldRecipe *initializers, AmaObjectOperation *operation) {
    bool owning = destination->form == AMA_FIELD_OWNING_LIST;
    if (source != NULL && (destination->form != source->form || destination->type_id != source->type_id)) {
        return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0);
    }
    if (destination->list == NULL) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    if (source != NULL && destination->list == source->list) { return source->continuation ? ama_status_make(AMA_INVALID_STATE,NULL,0) : ama_status_success(); }
    AmaStatus status = ama_status_success();
    if (owning && source != NULL) {
        status = ama_object_copy_capability(operation, destination->type_id);
        if (status.code == AMA_OK && source->list != NULL) { status = ama_transfer_list_preflight(operation, source->list, true); }
    }
    if (status.code == AMA_OK && owning) { status = ama_transfer_list_preflight(operation, destination->list, false); }
    if (status.code != AMA_OK) { return status; }
    AmaHandleList prepared = {0};
    status = ama_transfer_list_prepare(program, destination, source, initializers, operation, &prepared);
    if (status.code != AMA_OK) { return ama_transfer_list_discard(program, &prepared, owning, status); }
    if (owning && operation->first != NULL) {
        if (source != NULL) {
            for (AmaObjectWork *work = operation->first; work != NULL; work = work->next) { work->clone_root = operation->first; }
            ama_object_fix_references(operation, operation->first, (AmaHandle){0});
        }
    }
    AmaPoolCell *parent = NULL;
    status = ama_program_cell(program, destination->container, &parent);
    size_t old_count = destination->list->count;
    if (status.code == AMA_OK && owning && prepared.count > SIZE_MAX - (parent->metadata.lifetime.live_children - old_count)) {
        status = ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0);
    }
    if (status.code != AMA_OK) { return ama_transfer_list_discard(program, &prepared, owning, status); }
    status = ama_transfer_list_discard(program, destination->list, owning, status);
    if (owning) {
        parent->metadata.lifetime.live_children -= old_count;
        parent->metadata.lifetime.child_capacity -= old_count;
    }
    if (status.code != AMA_OK) { return ama_transfer_list_discard(program, &prepared, owning, status); }
    if (owning) {
        for (AmaObjectWork *work = operation->first; work != NULL; work = work->next) {
            if (work->parent == NULL) { work->cell->owner = destination->container; }
            AmaServiceState service = work->cell->metadata.service;
            (void)ama_pool_publish(&work->type->pool, work->handle);
            work->cell->metadata.service = service;
        }
        parent->metadata.lifetime.live_children += prepared.count;
        parent->metadata.lifetime.child_capacity += prepared.count;
    }
    *destination->list = prepared;
    return status;
}
