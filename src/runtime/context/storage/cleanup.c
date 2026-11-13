/**
 * @file
 * @brief Destroy every private context resource after checked identity invalidation.
 *
 * [RT-10](../../../../spec/implementation/runtime.md#rt-10) requires complete cleanup;
 * [LIFE-05](../../../../spec/implementation/native-interface.md#life-05) preserves primary
 * failures. Exclusive context admission permits terminal cleanup without borrowed views.
 */
#include "../internal.h"

/** Consume the newest surviving successful root first, independently of type/slot layout.
 * Whole-tree cleanup unlinks its root before releasing its cell. Stable-root replacement
 * retains this position; failed creations and owned children never register here.
 * No payload moves, emergency allocation, sequence wrap or C recursion is needed.
 */
static AmaStatus context_roots(AmaWorld *world, AmaStatus primary) {
    while (world->last_root != NULL) {
        AmaPoolCell *cell = world->last_root;
        AmaHandle root = {cell->root_type_id, cell->metadata.identity.slot_id,
            cell->metadata.identity.generation};
        primary = ama_cleanup_primary(primary, ama_object_destroy(world, root));
    }
    return primary;
}

/** Consume all owned roots, constants and initialized pools, then release descriptor scratch.
 * The record allocation itself belongs to the registry caller and is released separately.
 */
AmaStatus ama_context_cleanup(AmaContext *context, AmaStatus primary) {
    primary = ama_cleanup_primary(primary, context->status);
    context->world.failed = true;
    context->control.phase = AMA_RUNTIME_TERMINATED;
    primary = context_roots(&context->world, primary);
    primary = ama_program_constants_destroy(&context->program, primary);
    for (size_t index = 0; index < context->world.type_count; index++) {
        AmaPool *pool = &context->world.types[index]->pool;
        if (pool->allocator.release != NULL) { ama_pool_dispose(pool); }
    }
    source_arena_destroy(&context->arena);
    return primary;
}
