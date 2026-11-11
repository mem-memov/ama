#include <stdint.h>
#include <string.h>
#include "runtime/objects/internal.h"

AmaHandle *ama_object_handles(const AmaObjectType *type, void *payload, size_t field, size_t *count) {
    void *address = (unsigned char *)payload + type->fields[field].offset;
    if (ama_field_list(type->fields[field].kind)) {
        AmaHandleList *list = address;
        *count = list->count;
        return list->items;
    }
    *count = 1;
    return address;
}

AmaStatus ama_object_fields(AmaWorld *world, AmaObjectWork *work) {
    (void)world;
    if (work->recipe != NULL && work->recipe->fields != NULL && work->recipe->field_count != work->type->field_count) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    AmaConstructorKind kind = work->recipe == NULL ? AMA_CONSTRUCT_DEFAULT : work->recipe->constructor;
    return ama_lifetime_begin(&work->cell->metadata.lifetime, 0, kind);
}

AmaStatus ama_object_prepare_field(AmaObjectOperation *operation, AmaObjectWork *work) {
    AmaObjectType *type = work->type;
    size_t field = work->next_field;
    AmaFieldRecipe recipe = {0};
    bool explicit_fields = work->recipe != NULL && work->recipe->fields != NULL;
    if (explicit_fields) { recipe = work->recipe->fields[field]; }
    else if (type->defaults != NULL) { recipe = type->defaults[field]; }
    if (type->resolve_field != NULL && (!explicit_fields || !recipe.supplied)) {
        AmaStatus status = type->resolve_field(operation->world, &operation->arena,
            work->handle, field, &recipe, type->field_context);
        if (status.code != AMA_OK) { return status; }
    }
    const AmaObjectField *descriptor = &type->fields[field];
    size_t count = ama_field_list(descriptor->kind) ? recipe.count : 1;
    if (count > SIZE_MAX / sizeof(AmaHandle)
        || (ama_field_owns(descriptor->kind) && work->cell->metadata.lifetime.child_capacity > SIZE_MAX - count)) {
        return ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0);
    }
    void *payload = ama_pool_payload(&type->pool, work->cell);
    if (ama_field_list(descriptor->kind)) {
        AmaHandleList *list = (void *)((unsigned char *)payload + descriptor->offset);
        if (count != 0) {
            list->items = operation->world->allocator.allocate(operation->world->allocator.context, count * sizeof(AmaHandle));
            if (list->items == NULL) { return ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0); }
            memset(list->items, 0, count * sizeof(AmaHandle));
        }
        list->count = count;
        list->capacity = count;
    }
    if (ama_field_owns(descriptor->kind)) { work->cell->metadata.lifetime.child_capacity += count; }
    else if (recipe.references != NULL) {
        size_t actual;
        AmaHandle *handles = ama_object_handles(type, payload, field, &actual);
        for (size_t index = 0; index < count; index++) {
            if (recipe.references[index].generation != 0 && recipe.references[index].type_id != descriptor->type_id) {
                return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0);
            }
            handles[index] = recipe.references[index];
        }
    }
    work->field_recipe = recipe;
    work->field_ready = true;
    work->next_element = 0;
    return ama_status_success();
}

AmaHandle *ama_object_child(AmaObjectWork *work, size_t index, uint64_t *type_id, const AmaRecipe **recipe) {
    void *payload = ama_pool_payload(&work->type->pool, work->cell);
    for (size_t field = 0; field < work->type->field_count; field++) {
        if (!ama_field_owns(work->type->fields[field].kind)) { continue; }
        size_t count;
        AmaHandle *handles = ama_object_handles(work->type, payload, field, &count);
        if (index < count) {
            *type_id = work->type->fields[field].type_id;
            *recipe = NULL;
            return &handles[index];
        }
        index -= count;
    }
    return NULL;
}
