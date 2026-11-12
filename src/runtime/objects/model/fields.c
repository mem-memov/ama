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
