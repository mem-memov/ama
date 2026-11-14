/** @file
 * @brief Validate continuation controls before preparing any copied object.
 * [CONT-04](../../../../spec/language/continuations.md#cont-04) requires active
 * source collection identities to remain valid. [COPY-07](../../../../spec/language/values-and-references.md#copy-07)
 * supplies a complete protected ownership tree; operation records provide an
 * exact stackless traversal without modifying source cursors or allocating snapshots.
 */
#include "runtime/objects/internal.h"

/** Find the exact already protected source record, including its full generation. */
static AmaObjectProtection *ama_control_record(AmaObjectOperation *operation, AmaHandle handle) {
    for (AmaObjectProtection *record = operation->protected_first; record != NULL; record = record->next) {
        if (record->copy && ama_same_handle(record->handle, handle)) { return record; }
    }
    return NULL;
}

/** Validate one node before descending; successful subtree completion caches
 * only operation-local immutable-source proof, never a language-visible flag.
 */
static AmaStatus ama_control_validate(AmaObjectOperation *operation, AmaObjectProtection *record) {
    const AmaObjectControlHooks *hooks = record->type->controls;
    if (hooks == NULL || hooks->validate_continuation == NULL) { return ama_status_success(); }
    return hooks->validate_continuation(operation->world, record->handle,
        ama_pool_payload(&record->type->pool, record->cell), &record->previous, record->type->control_context);
}

AmaStatus ama_object_control_preflight(AmaObjectOperation *operation, AmaHandle root) {
    AmaObjectProtection *record = ama_control_record(operation, root);
    if (record == NULL) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    if (record->controls_validated) { return ama_status_success(); }
    record->control_parent = NULL;
    record->control_child = 0;
    AmaStatus status = ama_control_validate(operation, record);
    while (status.code == AMA_OK) {
        if (record->control_child < record->cell->metadata.lifetime.live_children) {
            AmaObjectWork work = {.type = record->type, .cell = record->cell};
            uint64_t type_id;
            const AmaRecipe *recipe;
            AmaHandle child = *ama_object_child(&work, record->control_child, &type_id, &recipe);
            AmaObjectProtection *next = ama_control_record(operation, child);
            if (next == NULL || child.type_id != type_id) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
            if (next->controls_validated) { record->control_child++; continue; }
            next->control_parent = record;
            next->control_child = 0;
            record = next;
            status = ama_control_validate(operation, record);
        } else {
            record->controls_validated = true;
            if (record->control_parent == NULL) { break; }
            record = record->control_parent;
            record->control_child++;
        }
    }
    return status;
}
