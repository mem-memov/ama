/**
 * @file
 * @brief Initialize clone field buffers and copy reference handles before identity fixups.
 * [LIST-13](../../../../spec/language/lists.md#list-13) requires exact occurrence retirement and replacement namespaces.
 *
 * [COPY-05](../../../../spec/language/values-and-references.md#copy-05)
 * requires fresh owning child slots and complete reference fixups before publication.
 */
#include "internal.h"

#include <stdint.h>
#include <string.h>

/** Allocate zeroed destination field storage from a protected source's shape.
 * Reference handles are copied pending complete-map fixups; owned handles remain
 * empty until child reservation. Buffers belong to the registered private work,
 * including after overflow, allocation or field-preparation failure.
 */
AmaStatus ama_object_clone_fields(AmaWorld *world, AmaObjectWork *work) {
    AmaPoolCell *source = NULL;
    AmaStatus status = ama_pool_lookup(&work->type->pool, work->source, true, &source);
    if (status.code != AMA_OK) { return status; }
    status = ama_lifetime_begin(&work->cell->metadata.lifetime,
        source->metadata.lifetime.live_children, AMA_CONSTRUCT_CLONE);
    if (status.code != AMA_OK) { return status; }
    void *original = ama_pool_payload(&work->type->pool, source);
    void *payload = ama_pool_payload(&work->type->pool, work->cell);
    for (size_t field = 0; field < work->type->field_count; field++) {
        size_t count = ama_object_field_count(work->type, original, field);
        if (ama_field_list(work->type->fields[field].kind)) {
            status = ama_object_list_prepare(world, work, field, count);
            if (status.code != AMA_OK) { return status; }
        }
        if (!ama_field_owns(work->type->fields[field].kind) && count != 0) {
            for (size_t index = 0; index < count; index++) {
                *ama_object_field_handle(work->type, payload, field, index) =
                    *ama_object_field_handle(work->type, original, field, index);
            }
        }
    }
    return ama_status_success();
}
