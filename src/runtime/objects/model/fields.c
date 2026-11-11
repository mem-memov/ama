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

static const AmaFieldRecipe *ama_object_recipes(const AmaObjectWork *work) {
    return work->recipe != NULL && work->recipe->fields != NULL ? work->recipe->fields : work->type->defaults;
}

AmaStatus ama_object_fields(AmaWorld *world, AmaObjectWork *work) {
    AmaObjectType *type = work->type;
    const AmaFieldRecipe *initial = ama_object_recipes(work);
    if (work->recipe != NULL && work->recipe->fields != NULL && work->recipe->field_count != type->field_count) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    void *payload = ama_pool_payload(&type->pool, work->cell);
    size_t children = 0;
    for (size_t field = 0; field < type->field_count; field++) {
        size_t count = ama_field_list(type->fields[field].kind) ? (initial == NULL ? 0 : initial[field].count) : 1;
        if (ama_field_owns(type->fields[field].kind)) {
            if (children > SIZE_MAX - count) { return ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0); }
            children += count;
        }
    }
    AmaConstructorKind kind = work->recipe == NULL ? AMA_CONSTRUCT_DEFAULT : work->recipe->constructor;
    AmaStatus status = ama_lifetime_begin(&work->cell->metadata.lifetime, children, kind);
    if (status.code != AMA_OK) { return status; }
    for (size_t field = 0; field < type->field_count; field++) {
        const AmaObjectField *descriptor = &type->fields[field];
        if (ama_field_list(descriptor->kind)) {
            AmaHandleList *list = (void *)((unsigned char *)payload + descriptor->offset);
            size_t count = initial == NULL ? 0 : initial[field].count;
            if (count > SIZE_MAX / sizeof(AmaHandle)) { return ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0); }
            if (count != 0) {
                list->items = world->allocator.allocate(world->allocator.context, count * sizeof(AmaHandle));
                if (list->items == NULL) { return ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0); }
                memset(list->items, 0, count * sizeof(AmaHandle));
            }
            list->count = count;
        }
        size_t count;
        AmaHandle *handles = ama_object_handles(type, payload, field, &count);
        if (!ama_field_owns(descriptor->kind) && initial != NULL && initial[field].references != NULL) {
            for (size_t index = 0; index < count; index++) {
                AmaHandle source = initial[field].references[index];
                if (source.generation != 0 && source.type_id != descriptor->type_id) {
                    return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0);
                }
                handles[index] = source;
            }
        }
    }
    return ama_status_success();
}

AmaHandle *ama_object_child(AmaObjectWork *work, size_t index, uint64_t *type_id, const AmaRecipe **recipe) {
    const AmaFieldRecipe *initial = ama_object_recipes(work);
    void *payload = ama_pool_payload(&work->type->pool, work->cell);
    for (size_t field = 0; field < work->type->field_count; field++) {
        if (!ama_field_owns(work->type->fields[field].kind)) { continue; }
        size_t count;
        AmaHandle *handles = ama_object_handles(work->type, payload, field, &count);
        if (index < count) {
            *type_id = work->type->fields[field].type_id;
            *recipe = initial == NULL || initial[field].children == NULL ? NULL : &initial[field].children[index];
            return &handles[index];
        }
        index -= count;
    }
    return NULL;
}
