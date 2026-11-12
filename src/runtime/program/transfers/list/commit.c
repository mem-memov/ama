/**
 * @file
 * @brief Commit staged list ownership after parent/count checks and old-value cleanup.
 *
 * [COPY-02](../../../../../spec/language/values-and-references.md#copy-02)
 * separates preparation failure from irreversible destruction/publication.
 */
#include <stdint.h>
#include "internal.h"

AmaStatus ama_transfer_list_commit(AmaProgram *program, AmaProgramValue *destination,
    AmaObjectOperation *operation, AmaHandleList *prepared) {
    bool owning = destination->form == AMA_FIELD_OWNING_LIST;
    AmaPoolCell *parent = NULL;
    AmaStatus status = ama_program_cell(program, destination->container, &parent);
    size_t old_count = destination->list->count;
    if (status.code == AMA_OK && owning && prepared->count > SIZE_MAX - (parent->metadata.lifetime.live_children - old_count)) {
        status = ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0);
    }
    if (status.code != AMA_OK) { return status; }
    status = ama_transfer_list_discard(program, destination->list, owning, status);
    if (owning) {
        parent->metadata.lifetime.live_children -= old_count;
        parent->metadata.lifetime.child_capacity -= old_count;
    }
    if (status.code != AMA_OK) { return status; }
    if (owning) {
        for (AmaObjectWork *work = operation->first; work != NULL; work = work->next) {
            if (work->parent == NULL) { work->cell->owner = destination->container; }
            AmaServiceState service = work->cell->metadata.service;
            (void)ama_pool_publish(&work->type->pool, work->handle);
            work->cell->metadata.service = service;
        }
        parent->metadata.lifetime.live_children += prepared->count;
        parent->metadata.lifetime.child_capacity += prepared->count;
    }
    *destination->list = *prepared;
    *prepared = (AmaHandleList){0};
    return status;
}
