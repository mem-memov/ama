/**
 * @file
 * @brief Clear active links across a terminal owned tree without allocation.
 *
 * [LIFE-05](../../../../spec/implementation/native-interface.md#life-05)
 * Use stable owner links and per-cell cursors, not recursive C calls or an emergency work allocation.
 */
#include "runtime/objects/internal.h"

/** Walk the initialized owned tree using owner links and cell cursors, clearing transient service controls without allocation or recursive C calls. This is terminal cleanup only. Component contract: [LIFE-05](../../../../spec/implementation/native-interface.md#life-05). */
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
