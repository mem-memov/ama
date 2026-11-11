#include <stdint.h>
#include <string.h>
#include "runtime/objects/internal.h"

AmaStatus ama_object_clone_fields(AmaWorld *world, AmaObjectWork *work) {
    AmaPoolCell *source = NULL;
    AmaStatus status = ama_pool_lookup(&work->type->pool, work->source, true, &source);
    if (status.code != AMA_OK) { return status; }
    status = ama_lifetime_begin(&work->cell->metadata.lifetime,
        source->metadata.lifetime.live_children, AMA_CONSTRUCT_CLONE);
    if (status.code != AMA_OK) { return status; }
    void *original = ama_pool_payload(&work->type->pool, source);
    void *payload = ama_pool_payload(&work->type->pool, work->cell);
    for (size_t field = 0; field < work->type->field_count; field++) {
        size_t count;
        AmaHandle *handles = ama_object_handles(work->type, original, field, &count);
        if (ama_field_list(work->type->fields[field].kind)) {
            AmaHandleList *list = (void *)((unsigned char *)payload + work->type->fields[field].offset);
            if (count > SIZE_MAX / sizeof(AmaHandle)) { return ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0); }
            if (count != 0) {
                list->items = world->allocator.allocate(world->allocator.context, count * sizeof(AmaHandle));
                if (list->items == NULL) { return ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0); }
                memset(list->items, 0, count * sizeof(AmaHandle));
            }
            list->refused = ((AmaHandleList *)((unsigned char *)original + work->type->fields[field].offset))->refused;
            list->outcome = ((AmaHandleList *)((unsigned char *)original + work->type->fields[field].offset))->outcome;
            list->count = count;
        list->capacity = count;
        }
        if (!ama_field_owns(work->type->fields[field].kind) && count != 0) {
            size_t destination_count;
            AmaHandle *destination = ama_object_handles(work->type, payload, field, &destination_count);
            memcpy(destination, handles, count * sizeof(AmaHandle));
        }
    }
    return ama_status_success();
}
