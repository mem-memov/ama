#include "runtime/objects/internal.h"

AmaStatus ama_object_destroy(AmaWorld *world, AmaHandle root) {
    if (world->operation_active) { return ama_status_make(AMA_BUSY, NULL, 0); }
    AmaObjectType *type = NULL;
    AmaPoolCell *cell = NULL;
    AmaStatus status = ama_object_lookup(world, root, world->failed, &type, &cell);
    if (status.code != AMA_OK) { return status; }
    status = cell->metadata.lifetime.phase == AMA_LIFETIME_FINALIZED && world->failed
        ? ama_status_success() : ama_instance_access(&cell->metadata, AMA_ACCESS_READ);
    if (status.code != AMA_OK) { return status; }
    world->operation_active = true;
    status = ama_object_cleanup(world, root, ama_status_success());
    world->operation_active = false;
    return status;
}
