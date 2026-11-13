/**
 * @file
 * @brief Finalize initialized resources and release owned slots iteratively.
 *
 * [LIFE-03](../../../../spec/implementation/native-interface.md#life-03)
 * Invalidate identity before native cleanup, preserve the first failure and continue through the reverse initialized prefix.
 * [ERROR-03](../../../../spec/implementation/diagnostics.md#error-03) requires
 * checking destroy transport after consuming its credit, so malformed zero-code
 * completion cannot hide a technical error or cause a retry.
 */
#include "runtime/objects/internal.h"

/** Release every owned/reference list handle buffer after child cleanup; referenced targets are never destroyed by releasing their array. */
static void ama_cleanup_buffers(AmaWorld *world, AmaObjectType *type, AmaPoolCell *cell) {
    unsigned char *payload = ama_pool_payload(&type->pool, cell);
    for (size_t field = 0; field < type->field_count; field++) {
        if (ama_field_list(type->fields[field].kind)) {
            AmaHandleList *list = (void *)(payload + type->fields[field].offset);
            world->allocator.release(world->allocator.context, list->items);
            *list = (AmaHandleList){0};
        }
    }
}

/** Iteratively invalidate and finalize one initialized tree, native resource first and children in reverse. Continue after destructor errors, marking the world terminal; optionally retain the root slot for replacement. */
AmaStatus ama_object_cleanup_keep(AmaWorld *world, AmaHandle root, AmaStatus primary, bool keep_root) {
    AmaHandle current = root;
    for (;;) {
        AmaObjectType *type = NULL;
        AmaPoolCell *cell = NULL;
        AmaStatus found = ama_object_lookup(world, current, true, &type, &cell);
        if (found.code != AMA_OK) { return ama_cleanup_primary(primary, found); }
        AmaLifetime *lifetime = &cell->metadata.lifetime;
        if (!lifetime->cleanup_active && lifetime->phase != AMA_LIFETIME_FINALIZED) {
            if (cell->metadata.identity.live) { (void)ama_pool_invalidate(&type->pool, current); }
            if (lifetime->phase == AMA_LIFETIME_FRESH) {
                lifetime->phase = AMA_LIFETIME_CONSTRUCTING;
            }
            (void)ama_lifetime_begin_cleanup(lifetime);
            if (ama_lifetime_take_native(lifetime)) {
                void *payload = ama_pool_payload(&type->pool, cell);
                AmaStatus destroyed = ama_status_checked(type->hooks.destroy((unsigned char *)payload + type->native_offset));
                ama_object_report(world, destroyed);
                if (destroyed.code != AMA_OK) { world->failed = true; }
                primary = ama_cleanup_primary(primary, destroyed);
            }
        }
        size_t child;
        if (ama_lifetime_next_child(lifetime, &child)) {
            AmaObjectWork work = {.type = type, .cell = cell};
            uint64_t child_type;
            const AmaRecipe *recipe;
            current = *ama_object_child(&work, child, &child_type, &recipe);
            continue;
        }
        AmaHandle parent = cell->owner;
        ama_cleanup_buffers(world, type, cell);
        if (lifetime->cleanup_active) { (void)ama_lifetime_end_cleanup(lifetime); }
        if (!keep_root || !ama_same_handle(current, root)) {
            ama_object_root_remove(world, cell);
            (void)ama_pool_release(&type->pool, current);
        }
        if (ama_same_handle(current, root)) { return primary; }
        current = parent;
    }
}

/** Clean the complete initialized tree and release its root slot, retaining the supplied primary failure over secondary cleanup errors. */
AmaStatus ama_object_cleanup(AmaWorld *world, AmaHandle root, AmaStatus primary) {
    return ama_object_cleanup_keep(world, root, primary, false);
}
