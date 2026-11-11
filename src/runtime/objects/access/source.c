#include "runtime/objects/internal.h"

AmaStatus ama_object_source_lookup(AmaObjectOperation *operation, AmaHandle handle,
    AmaObjectType **type, AmaPoolCell **cell) {
    AmaStatus status = ama_object_lookup(operation->world, handle, false, type, cell);
    if (status.code != AMA_STALE_REFERENCE) { return status; }
    for (AmaObjectWork *work = operation->first; work != NULL; work = work->next) {
        if (ama_same_handle(work->handle, handle) && work->cell->metadata.lifetime.phase == AMA_LIFETIME_LIVE) {
            *type = work->type;
            *cell = work->cell;
            return ama_status_success();
        }
    }
    return status;
}
