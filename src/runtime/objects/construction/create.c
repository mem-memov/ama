/**
 * @file
 * @brief Own a guarded object-creation operation and publish only its completed tree.
 *
 * [LIFE-01](../../../../spec/implementation/native-interface.md#life-01) requires explicit lifecycle ordering.
 * The caller output changes only on success; all partial scratch/protection is released.
 * [RT-02](../../../../spec/implementation/runtime.md#rt-02) preserves stable instance storage.
 */
#include "internal.h"

/** Guard one synchronous creation, release all temporary protections on exit,
 * and expose output only after complete publication. Recipes are borrowed.
 * Refuse reentry or terminal worlds before changing their state.
 */
AmaStatus ama_object_create(AmaWorld *world, uint64_t id, const AmaRecipe *recipe, AmaHandle *output) {
    if (world->operation_active || world->failed) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    world->operation_active = true;
    AmaObjectOperation operation = {.world = world};
    source_arena_init(&operation.arena, world->allocator);
    AmaHandle handle = {0};
    AmaStatus status = ama_object_build(&operation, id, recipe, &handle);
    if (status.code == AMA_OK) {
        ama_construction_publish(&operation);
        ama_object_root_append(world, handle, operation.first->cell);
        *output = handle;
    }
    ama_object_unprotect(&operation);
    source_arena_destroy(&operation.arena);
    world->operation_active = false;
    return status;
}
