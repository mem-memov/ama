/**
 * @file
 * @brief Drive iterative ownership construction, payload completion and clone fixups.
 *
 * [LIFE-01](../../../../spec/implementation/native-interface.md#life-01) requires explicit lifecycle ordering.
 * A parent is credited only after a child completes; failure unwinds the exact current ancestry.
 * [RT-02](../../../../spec/implementation/runtime.md#rt-02) preserves stable instance storage.
 */
#include "internal.h"

/** Build privately under the caller-held world guard. Scratch and protections
 * belong to operation; output changes only after all constructors and fixups.
 * Any failure is reported once and unwinds the reserved current ancestry.
 */
AmaStatus ama_object_build(AmaObjectOperation *operation, uint64_t id, const AmaRecipe *recipe, AmaHandle *output) {
    AmaObjectWork *work = NULL;
    AmaStatus status = ama_construction_reserve(operation, id, recipe, (AmaHandle){0}, NULL, &work);
    AmaObjectWork *root = work;
    while (status.code == AMA_OK) {
        bool skipped;
        status = ama_construction_prepare_next(operation, work, &skipped);
        if (status.code != AMA_OK) { break; }
        if (skipped) { continue; }
        bool clone_source = work->source.generation != 0;
        if ((clone_source && work->next_child < work->cell->metadata.lifetime.child_capacity)
            || (!clone_source && work->next_field < work->type->field_count)) {
            status = ama_construction_descend(operation, &work);
        } else {
            status = ama_construction_payload(operation, work);
            if (status.code != AMA_OK || work->parent == NULL) { break; }
            work = work->parent;
            work->next_child++;
            work->next_element++;
            status = ama_lifetime_child_ready(&work->cell->metadata.lifetime);
        }
    }
    if (status.code != AMA_OK) {
        return ama_construction_unwind(operation, work, status);
    }
    for (AmaObjectWork *record = operation->first; record != NULL; record = record->next) {
        if (record->clone_root == record) { ama_object_fix_references(operation, record, (AmaHandle){0}); }
    }
    *output = root->handle;
    return ama_status_success();
}
