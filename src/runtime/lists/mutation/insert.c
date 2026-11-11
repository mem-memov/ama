#include <stdint.h>
#include <string.h>
#include "internal.h"

AmaStatus ama_list_insert(AmaListMutation *mutation, size_t position, AmaHandle source) {
    AmaWorld *world = mutation->objects.world;
    AmaHandleList *list = mutation->list;
    if (list->count == SIZE_MAX || (mutation->owning && mutation->parent->metadata.lifetime.live_children == SIZE_MAX)) {
        return ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0);
    }
    AmaStatus status = ama_handle_list_reserve(world->allocator, list, list->count + 1);
    if (status.code != AMA_OK) { return status; }
    AmaHandle element = source;
    if (mutation->owning) {
        AmaRecipe recipe = {.constructor = source.generation == 0 ? AMA_CONSTRUCT_DEFAULT : AMA_CONSTRUCT_CLONE, .source = source};
        status = ama_object_build(&mutation->objects, mutation->type, &recipe, &element);
        if (status.code != AMA_OK) { return status; }
        for (AmaObjectWork *work = mutation->objects.first; work != NULL; work = work->next) {
            if (work->parent == NULL) { work->cell->owner = mutation->owner; }
            AmaServiceState service = work->cell->metadata.service;
            (void)ama_pool_publish(&work->type->pool, work->handle);
            if (work->source.generation != 0) { work->cell->metadata.service = service; }
        }
        mutation->parent->metadata.lifetime.live_children++;
        mutation->parent->metadata.lifetime.child_capacity++;
    }
    memmove(list->items + position + 1, list->items + position, (list->count - position) * sizeof(*list->items));
    list->items[position] = element;
    list->count++;
    return ama_status_success();
}
