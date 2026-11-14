/** @file
 * @brief Copy compiler continuation cells only after complete destination fields.
 * [CONT-04](../../../../spec/language/continuations.md#cont-04) maps saved cursor
 * and binding identities through the same ownership boundary as declared refs.
 * [COPY-03](../../../../spec/language/values-and-references.md#copy-03) leaves
 * ordinary copies' fresh zeroed control storage inactive. These are compiler
 * metadata hooks, never native resource protocols or Ama body dispatch.
 */
#include "internal.h"

AmaStatus ama_clone_control_map(AmaObjectOperation *operation, AmaObjectWork *work,
    const AmaCloneMap *map, bool continuation) {
    const AmaObjectControlHooks *hooks = work->type->controls;
    if (hooks == NULL) { return ama_status_success(); }
    void *destination = ama_pool_payload(&work->type->pool, work->cell);
    if (continuation && hooks->copy_continuation != NULL) {
        AmaObjectType *source_type = NULL;
        AmaPoolCell *source = NULL;
        AmaStatus status = ama_object_source_lookup(operation, work->source, &source_type, &source);
        if (status.code != AMA_OK) { return status; }
        const AmaObjectControlCopy copy = {.world = operation->world,
            .source = work->source, .destination = work->identity,
            .source_storage = ama_pool_payload(&source_type->pool, source), .destination_storage = destination,
            .map = ama_clone_reference_map, .map_context = map};
        status = hooks->copy_continuation(&copy, work->type->control_context);
        if (status.code != AMA_OK) { return status; }
    }
    return hooks->map_references == NULL ? ama_status_success()
        : hooks->map_references(operation->world, work->type->pool.type_id, destination,
            ama_clone_reference_map, map, work->type->control_context);
}
