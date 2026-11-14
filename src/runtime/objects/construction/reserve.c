/**
 * @file
 * @brief Reserve one private instance and register its construction work record.
 *
 * [LIFE-01](../../../../spec/implementation/native-interface.md#life-01) requires explicit lifecycle ordering.
 * Failure exposes a work record only after its slot is reserved for cleanup.
 * [LIFE-04](../../../../spec/implementation/native-interface.md#life-04)
 * gives a prepared replacement root the existing destination identity before
 * initializers; its private handle still owns cleanup storage.
 * [RT-02](../../../../spec/implementation/runtime.md#rt-02) preserves stable instance storage.
 */
#include "internal.h"

AmaStatus ama_construction_reserve(AmaObjectOperation *operation, uint64_t id, const AmaRecipe *recipe,
    AmaHandle source, AmaObjectWork *parent, AmaObjectWork **output) {
    AmaObjectType *type = ama_world_type(operation->world, id);
    if (type == NULL) { return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0); }
    operation->world->construction_operation = parent == NULL ? type->source_operation
        : parent->source.generation != 0 ? parent->operation : parent->field_recipe.operation;
    AmaObjectWork *work = source_arena_allocate(&operation->arena, sizeof(*work));
    if (work == NULL) { return ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0); }
    *work = (AmaObjectWork){.parent = parent, .type = type, .recipe = recipe, .source = source,
        .continuation = recipe != NULL && recipe->continuation,
        .operation = parent == NULL ? type->source_operation : parent->source.generation != 0
            ? parent->operation : parent->field_recipe.operation};
    if (recipe != NULL && recipe->constructor == AMA_CONSTRUCT_CLONE) { work->source = recipe->source; }
    if (work->source.generation != 0) {
        if (work->source.type_id != id) { return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0); }
        work->clone_root = parent != NULL && parent->source.generation != 0 ? parent->clone_root : work;
        if (work->clone_root == work) {
            AmaStatus protected = ama_object_preflight(operation, work->source, true,
                recipe != NULL && recipe->continuation);
            if (protected.code != AMA_OK) { return protected; }
        }
    } else if (recipe != NULL && recipe->constructor == AMA_CONSTRUCT_CLONE && recipe->native_source == NULL) {
        return ama_status_make(AMA_EMPTY_REFERENCE, NULL, 0);
    } else if (recipe != NULL && recipe->constructor == AMA_CONSTRUCT_CLONE) {
        AmaStatus capability = ama_object_copy_capability(operation, id);
        if (capability.code != AMA_OK) { return capability; }
    }
    AmaStatus status = ama_pool_reserve(&type->pool, &work->handle, &work->cell);
    if (status.code != AMA_OK) { return status; }
    work->identity = parent == NULL && operation->root_identity.generation != 0
        ? operation->root_identity : work->handle;
    if (parent != NULL) { work->cell->owner = parent->handle; }
    else if (operation->root_payload != NULL) { work->cell->preparation_payload = operation->root_payload; }
    if (operation->last == NULL) { operation->first = work; }
    else { operation->last->next = work; }
    operation->last = work;
    *output = work;
    return work->source.generation == 0 ? ama_object_fields(operation->world, work)
        : ama_object_clone_fields(operation->world, work);
}
