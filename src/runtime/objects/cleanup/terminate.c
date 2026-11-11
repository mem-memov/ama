#include "runtime/objects/internal.h"

AmaStatus ama_object_end_active_links(AmaWorld *world, AmaHandle root) {
    AmaHandle current = root;
    AmaObjectType *type;
    AmaPoolCell *cell;
    AmaStatus status = ama_object_lookup(world, current, true, &type, &cell);
    if (status.code != AMA_OK) { return status; }
    cell->traversal_child = 0;
    cell->metadata.service = (AmaServiceState){0};
    for (;;) {
        if (cell->traversal_child < cell->metadata.lifetime.live_children) {
            AmaObjectWork work = {.type = type, .cell = cell};
            uint64_t type_id;
            const AmaRecipe *recipe;
            current = *ama_object_child(&work, cell->traversal_child++, &type_id, &recipe);
            status = ama_object_lookup(world, current, true, &type, &cell);
            if (status.code != AMA_OK) { return status; }
            cell->traversal_child = 0;
            cell->metadata.service = (AmaServiceState){0};
        } else {
            if (ama_same_handle(current, root)) { return ama_status_success(); }
            current = cell->owner;
            status = ama_object_lookup(world, current, true, &type, &cell);
            if (status.code != AMA_OK) { return status; }
        }
    }
}
