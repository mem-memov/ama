/**
 * @file
 * @brief Reserve and register private clone identities before preparing their fields.
 *
 * [COPY-05](../../../../spec/language/values-and-references.md#copy-05)
 * requires one exact source/destination association per owned instance.
 * Output work is exposed only after reservation; later failures still require cleanup.
 */
#include "internal.h"

AmaStatus ama_clone_reserve(AmaObjectOperation *operation, AmaHandle source,
    AmaObjectWork *parent, void *root_payload, AmaHandle final_identity, AmaObjectWork **output) {
    AmaObjectType *type = ama_world_type(operation->world, source.type_id);
    AmaObjectWork *work = source_arena_allocate(&operation->arena, sizeof(*work));
    if (work == NULL) { return ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0); }
    *work = (AmaObjectWork){.type = type, .parent = parent, .source = source};
    work->clone_root = parent == NULL ? work : parent->clone_root;
    AmaStatus status = ama_pool_reserve(&type->pool, &work->handle, &work->cell);
    if (status.code != AMA_OK) { return status; }
    work->identity = final_identity.generation == 0 ? work->handle : final_identity;
    if (parent != NULL) { work->cell->owner = parent->handle; }
    if (root_payload != NULL) { work->cell->preparation_payload = root_payload; }
    if (operation->last == NULL) { operation->first = work; }
    else { operation->last->next = work; }
    operation->last = work;
    *output = work;
    return ama_object_clone_fields(operation->world, work);
}
