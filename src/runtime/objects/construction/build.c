#include "runtime/objects/internal.h"

static AmaStatus reserve(AmaObjectOperation *operation, uint64_t id, const AmaRecipe *recipe,
    AmaObjectWork *parent, AmaObjectWork **output) {
    AmaObjectType *type = ama_world_type(operation->world, id);
    if (type == NULL) { return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0); }
    AmaObjectWork *work = source_arena_allocate(&operation->arena, sizeof(*work));
    if (work == NULL) { return ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0); }
    *work = (AmaObjectWork){.parent = parent, .type = type, .recipe = recipe};
    AmaStatus status = ama_pool_reserve(&type->pool, &work->handle, &work->cell);
    if (status.code != AMA_OK) { return status; }
    if (parent != NULL) { work->cell->owner = parent->handle; }
    if (operation->last == NULL) { operation->first = work; }
    else { operation->last->next = work; }
    operation->last = work;
    *output = work;
    return ama_object_fields(operation->world, work);
}

static AmaStatus native(AmaObjectWork *work) {
    AmaObjectType *type = work->type;
    void *payload = (unsigned char *)ama_pool_payload(&type->pool, work->cell) + type->native_offset;
    AmaConstructorKind kind = work->recipe == NULL ? AMA_CONSTRUCT_DEFAULT : work->recipe->constructor;
    AmaStatus status = ama_status_success();
    if (kind == AMA_CONSTRUCT_DEFAULT) {
        if (type->hooks.initialize != NULL) { status = type->hooks.initialize(payload); }
    } else if (kind == AMA_CONSTRUCT_LITERAL && type->hooks.literal != NULL) {
        status = type->hooks.literal(payload, work->recipe->literal);
    } else { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    return status.code == AMA_OK ? ama_lifetime_finish(&work->cell->metadata.lifetime,
        type->hooks.initialize != NULL) : status;
}

AmaStatus ama_object_build(AmaObjectOperation *operation, uint64_t id, const AmaRecipe *recipe, AmaHandle *output) {
    AmaObjectWork *work = NULL;
    AmaStatus status = reserve(operation, id, recipe, NULL, &work);
    while (status.code == AMA_OK) {
        if (work->next_child < work->cell->metadata.lifetime.child_capacity) {
            uint64_t child_type;
            const AmaRecipe *child_recipe;
            AmaHandle *location = ama_object_child(work, work->next_child, &child_type, &child_recipe);
            status = reserve(operation, child_type, child_recipe, work, &work);
            if (work != NULL) { *location = work->handle; }
        } else {
            status = native(work);
            if (status.code != AMA_OK || work->parent == NULL) { break; }
            work = work->parent;
            work->next_child++;
            status = ama_lifetime_child_ready(&work->cell->metadata.lifetime);
        }
    }
    if (status.code != AMA_OK) {
        ama_object_report(operation->world, status);
        while (work != NULL) {
            status = ama_object_cleanup(operation->world, work->handle, status);
            work = work->parent;
        }
        return status;
    }
    *output = operation->first->handle;
    return ama_status_success();
}

AmaStatus ama_object_create(AmaWorld *world, uint64_t id, const AmaRecipe *recipe, AmaHandle *output) {
    if (world->operation_active || world->failed) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    world->operation_active = true;
    AmaObjectOperation operation = {.world = world};
    source_arena_init(&operation.arena, world->allocator);
    AmaHandle handle = {0};
    AmaStatus status = ama_object_build(&operation, id, recipe, &handle);
    if (status.code == AMA_OK) {
        for (AmaObjectWork *work = operation.first; work != NULL; work = work->next) {
            (void)ama_pool_publish(&work->type->pool, work->handle);
        }
        *output = handle;
    }
    source_arena_destroy(&operation.arena);
    world->operation_active = false;
    return status;
}
