#include <string.h>
#include "internal.h"

AmaStatus ama_list_remove(AmaListMutation *mutation, size_t position, size_t count) {
    AmaHandleList *list = mutation->list;
    AmaStatus status = ama_status_success();
    if (mutation->owning) {
        /* Preflight has covered every affected tree before the first destructor. */
        for (size_t index = count; index != 0;) {
            status = ama_cleanup_primary(status, ama_object_cleanup(mutation->objects.world,
                list->items[position + --index], ama_status_success()));
        }
        mutation->parent->metadata.lifetime.live_children -= count;
        mutation->parent->metadata.lifetime.child_capacity -= count;
    }
    if (count != 0) {
        memmove(list->items + position, list->items + position + count,
            (list->count - position - count) * sizeof(*list->items));
        list->count -= count;
    }
    return status;
}
