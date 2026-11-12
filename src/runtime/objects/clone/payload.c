/**
 * @file
 * @brief Clone a native payload and project the protected source service state.
 *
 * [CONT-04](../../../../spec/language/continuations.md#cont-04)
 * permits saved positions only in explicit continuation copies.
 * Native clone success marks its payload initialized before later state validation.
 */
#include "internal.h"

/** Clone exactly one native payload after all children complete. The source and
 * its saved pre-acquisition state must belong to operation's protections. Failure
 * before successful native initialization leaves no destructor obligation; a
 * later service error retains that obligation for the caller's unwind.
 */
AmaStatus ama_object_clone_payload(AmaObjectOperation *operation, AmaObjectWork *work, bool continuation) {
    AmaPoolCell *source = NULL;
    AmaStatus status = ama_pool_lookup(&work->type->pool, work->source, true, &source);
    if (status.code != AMA_OK) { return status; }
    if (work->type->hooks.clone != NULL) {
        void *destination = ama_pool_payload(&work->type->pool, work->cell);
        void *original = ama_pool_payload(&work->type->pool, source);
        status = work->type->hooks.clone((unsigned char *)destination + work->type->native_offset,
            (unsigned char *)original + work->type->native_offset);
        if (status.code != AMA_OK) { return status; }
    }
    status = ama_lifetime_finish(&work->cell->metadata.lifetime, work->type->hooks.initialize != NULL);
    for (AmaObjectProtection *record = operation->protected_first; record != NULL; record = record->next) {
        if (ama_same_handle(record->handle, work->source)) {
            return ama_service_clone(&record->previous, continuation, &work->cell->metadata.service);
        }
    }
    return status.code == AMA_OK ? ama_status_make(AMA_INVALID_STATE, NULL, 0) : status;
}
