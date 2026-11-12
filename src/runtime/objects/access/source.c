/**
 * @file
 * @brief Resolve public sources or completed private work in the same operation.
 *
 * [COPY-02](../../../../spec/language/values-and-references.md#copy-02)
 * Only exact registered work identities allow private access; unrelated reserved slots remain inaccessible.
 */
#include "runtime/objects/internal.h"

/** Resolve normal public storage first; only a stale public lookup may match exact completed private work owned by this operation. Never expose unrelated reserved storage. */
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
