/**
 * @file
 * @brief Publish fully constructed pool slots while retaining explicit copied service state.
 *
 * [LIFE-01](../../../../spec/implementation/native-interface.md#life-01) requires explicit lifecycle ordering.
 * All constructors and fixups precede visibility; source-linked copies keep their prepared state.
 * [RT-02](../../../../spec/implementation/runtime.md#rt-02) preserves stable instance storage.
 */
#include "internal.h"

void ama_construction_publish(AmaObjectOperation *operation) {
    for (AmaObjectWork *work = operation->first; work != NULL; work = work->next) {
        AmaServiceState service = work->cell->metadata.service;
        (void)ama_pool_publish(&work->type->pool, work->handle);
        if (work->source.generation != 0) { work->cell->metadata.service = service; }
    }
}
