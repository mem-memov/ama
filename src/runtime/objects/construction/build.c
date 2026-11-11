#include "runtime/objects/internal.h"

static AmaStatus ama_construction_reserve(AmaObjectOperation *operation, uint64_t id, const AmaRecipe *recipe,
    AmaHandle source, AmaObjectWork *parent, AmaObjectWork **output) {
    AmaObjectType *type = ama_world_type(operation->world, id);
    if (type == NULL) { return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0); }
    operation->world->construction_operation = parent == NULL ? type->source_operation
        : parent->source.generation != 0 ? parent->operation : parent->field_recipe.operation;
    AmaObjectWork *work = source_arena_allocate(&operation->arena, sizeof(*work));
    if (work == NULL) { return ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0); }
    *work = (AmaObjectWork){.parent = parent, .type = type, .recipe = recipe, .source = source,
        .operation = parent == NULL ? type->source_operation : parent->source.generation != 0
            ? parent->operation : parent->field_recipe.operation};
    if (recipe != NULL && recipe->constructor == AMA_CONSTRUCT_CLONE) { work->source = recipe->source; }
    if (work->source.generation != 0) {
        if (work->source.type_id != id) { return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0); }
        work->clone_root = parent != NULL && parent->source.generation != 0 ? parent->clone_root : work;
        if (work->clone_root == work) {
            AmaStatus protected = ama_object_preflight(operation, work->source, true, false);
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
    if (parent != NULL) { work->cell->owner = parent->handle; }
    else if (operation->root_payload != NULL) { work->cell->preparation_payload = operation->root_payload; }
    if (operation->last == NULL) { operation->first = work; }
    else { operation->last->next = work; }
    operation->last = work;
    *output = work;
    return work->source.generation == 0 ? ama_object_fields(operation->world, work)
        : ama_object_clone_fields(operation->world, work);
}

static AmaStatus ama_construction_payload(AmaObjectOperation *operation, AmaObjectWork *work) {
    operation->world->construction_operation = work->operation;
    if (work->source.generation != 0) { return ama_object_clone_payload(operation, work, work->clone_root->recipe != NULL
        && work->clone_root->recipe->continuation); }
    AmaObjectType *type = work->type;
    void *payload = (unsigned char *)ama_pool_payload(&type->pool, work->cell) + type->native_offset;
    AmaConstructorKind kind = work->recipe == NULL ? AMA_CONSTRUCT_DEFAULT : work->recipe->constructor;
    AmaStatus status = ama_status_success();
    if (kind == AMA_CONSTRUCT_DEFAULT) {
        if (type->hooks.initialize != NULL) { status = type->hooks.initialize(payload); }
    } else if (kind == AMA_CONSTRUCT_LITERAL && type->hooks.literal != NULL) {
        status = type->hooks.literal(payload, work->recipe->literal);
    } else if (kind == AMA_CONSTRUCT_CLONE && work->recipe->native_source != NULL) {
        if (type->hooks.initialize != NULL && type->hooks.clone == NULL) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
        if (type->hooks.clone != NULL) { status = type->hooks.clone(payload, work->recipe->native_source); }
    } else { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    if (status.code == AMA_OK) { work->cell->metadata.service.activity = AMA_INSTANCE_READY; }
    return status.code == AMA_OK ? ama_lifetime_finish(&work->cell->metadata.lifetime,
        type->hooks.initialize != NULL) : status;
}

AmaStatus ama_object_build(AmaObjectOperation *operation, uint64_t id, const AmaRecipe *recipe, AmaHandle *output) {
    AmaObjectWork *work = NULL;
    AmaStatus status = ama_construction_reserve(operation, id, recipe, (AmaHandle){0}, NULL, &work);
    AmaObjectWork *root = work;
    while (status.code == AMA_OK) {
        bool clone_source = work->source.generation != 0;
        if (!clone_source && work->next_field < work->type->field_count && !work->field_ready) {
            status = ama_object_prepare_field(operation, work);
            if (status.code != AMA_OK) { break; }
        }
        if (!clone_source && work->next_field < work->type->field_count) {
            const AmaObjectField *field = &work->type->fields[work->next_field];
            size_t count;
            (void)ama_object_handles(work->type, ama_pool_payload(&work->type->pool, work->cell), work->next_field, &count);
            if (!ama_field_owns(field->kind) || work->next_element == count) {
                work->next_field++;
                work->field_ready = false;
                continue;
            }
        }
        if ((clone_source && work->next_child < work->cell->metadata.lifetime.child_capacity)
            || (!clone_source && work->next_field < work->type->field_count)) {
            uint64_t child_type;
            const AmaRecipe *child_recipe;
            AmaHandle *location;
            if (clone_source) { location = ama_object_child(work, work->next_child, &child_type, &child_recipe); }
            else {
                size_t count;
                AmaHandle *handles = ama_object_handles(work->type, ama_pool_payload(&work->type->pool, work->cell), work->next_field, &count);
                location = &handles[work->next_element];
                child_type = work->type->fields[work->next_field].type_id;
                child_recipe = work->field_recipe.children == NULL ? NULL : &work->field_recipe.children[work->next_element];
            }
            AmaHandle child_source = {0};
            if (work->source.generation != 0) {
                AmaPoolCell *original = NULL;
                (void)ama_pool_lookup(&work->type->pool, work->source, true, &original);
                AmaObjectWork source_work = {.type = work->type, .cell = original};
                child_source = *ama_object_child(&source_work, work->next_child, &child_type, &child_recipe);
                child_recipe = NULL;
            }
            AmaObjectWork *child_work = NULL;
            status = ama_construction_reserve(operation, child_type, child_recipe, child_source, work, &child_work);
            if (child_work != NULL) {
                *location = child_work->handle;
                work = child_work;
            }
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
        ama_object_report(operation->world, status);
        while (work != NULL) {
            status = ama_object_cleanup(operation->world, work->handle, status);
            work = work->parent;
        }
        return status;
    }
    for (AmaObjectWork *record = operation->first; record != NULL; record = record->next) {
        if (record->clone_root == record) { ama_object_fix_references(operation, record, (AmaHandle){0}); }
    }
    *output = root->handle;
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
            AmaServiceState service = work->cell->metadata.service;
            (void)ama_pool_publish(&work->type->pool, work->handle);
            if (work->source.generation != 0) { work->cell->metadata.service = service; }
        }
        *output = handle;
    }
    ama_object_unprotect(&operation);
    source_arena_destroy(&operation.arena);
    world->operation_active = false;
    return status;
}
