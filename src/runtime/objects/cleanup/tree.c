#include "runtime/objects/internal.h"

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
                AmaStatus destroyed = type->hooks.destroy((unsigned char *)payload + type->native_offset);
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
            (void)ama_pool_release(&type->pool, current);
        }
        if (ama_same_handle(current, root)) { return primary; }
        current = parent;
    }
}

AmaStatus ama_object_cleanup(AmaWorld *world, AmaHandle root, AmaStatus primary) {
    return ama_object_cleanup_keep(world, root, primary, false);
}
