/** @file
 * @brief Own real self-initializing program trees and guarded staged reconstruction with exact old-value snapshots.
 */
static AmaHandle destination;
/** Snapshot only live logical values and borrowed addresses that remain valid if preparation fails. */
typedef struct {
    RootValue *storage;
    void *resource;
    uintptr_t resource_identity;
    size_t number;
    AmaHandle left;
    AmaHandle right;
    AmaHandle selected;
    AmaHandle self;
    AmaServiceState services[3];
} PreviousTree;
/** Resolve one checked cell while its object identity remains owned. */
static AmaPoolCell *reconstruction_cell(AmaHandle handle) {
    AmaObjectType *type = ama_world_type(&world, handle.type_id);
    AmaPoolCell *cell = NULL;
    AmaStatus status = ama_pool_lookup(&type->pool, handle, false, &cell);
    assert(status.code == AMA_OK);
    return cell;
}
/** Initialize a scalar-owned tree with real self recipes and available retained execution state. */
static void setup_reconstruction(void) {
    setup_construction_program();
    AmaStatus status = ama_program_start(&program, 2, &destination);
    assert(status.code == AMA_OK && constructed == 3);
    assert_constructed_self_tree(destination);
    RootValue *root = object_payload(destination);
    AmaHandle handles[] = {destination, root->left, root->right};
    for (size_t index = 0; index < 3; index++) {
        AmaServiceState *service = &reconstruction_cell(handles[index])->metadata.service;
        service->activity = AMA_INSTANCE_YIELDED;
        service->resume = (AmaResumePosition){true, index + 50};
        service->current_operation = index + 60;
        service->last_refusal = ama_status_make(AMA_BUSY, "earlier", 7);
    }
}
/** Capture old tree values before any private preparation or protection changes. */
static PreviousTree snapshot_tree(void) {
    RootValue *root = object_payload(destination);
    PreviousTree previous = {.storage = root, .resource = root->native.resource,
        .resource_identity = (uintptr_t)root->native.resource, .number = root->native.number,
        .left = root->left, .right = root->right, .selected = root->selected, .self = root->self};
    AmaHandle handles[] = {destination, root->left, root->right};
    for (size_t index = 0; index < 3; index++) { previous.services[index] = reconstruction_cell(handles[index])->metadata.service; }
    return previous;
}
/** Failed preparation preserves values, handles, native resources and all retained service controls. */
static void assert_previous_tree(const PreviousTree *previous) {
    RootValue *root = object_payload(destination);
    assert(root == previous->storage && root->native.resource == previous->resource && root->native.number == previous->number);
    assert(ama_handle_equal(root->left, previous->left) && ama_handle_equal(root->right, previous->right));
    assert(ama_handle_equal(root->selected, previous->selected) && ama_handle_equal(root->self, previous->self));
    AmaHandle handles[] = {destination, root->left, root->right};
    for (size_t index = 0; index < 3; index++) {
        const AmaServiceState *actual = &reconstruction_cell(handles[index])->metadata.service;
        const AmaServiceState *expected = &previous->services[index];
        assert(actual->activity == expected->activity && actual->resume.valid == expected->resume.valid);
        assert(actual->resume.selector == expected->resume.selector && actual->current_operation == expected->current_operation);
        assert(actual->last_refusal.code == expected->last_refusal.code && actual->last_refusal.message == expected->last_refusal.message);
        assert(actual->last_refusal.message_length == expected->last_refusal.message_length);
        assert(!actual->return_link.attached && ama_handle_is_empty(actual->outgoing));
    }
    assert(!world.failed && !world.operation_active && world.construction_work == NULL);
    assert(root_type.pool.live_count == 1 && leaf_type.pool.live_count == 2);
}
/** Own one internal guarded replacement operation; release all protections/scratch and require callback authority to have ended on every outcome. */
static AmaStatus reconstruct(const AmaRecipe *recipe) {
    assert(!world.operation_active && world.construction_work == NULL);
    world.operation_active = true;
    AmaObjectOperation operation = {.world = &world};
    source_arena_init(&operation.arena, world.allocator);
    AmaStatus status = ama_object_reconstruct_in(&operation, destination, recipe);
    assert(world.construction_work == NULL);
    ama_object_unprotect(&operation);
    source_arena_destroy(&operation.arena);
    world.operation_active = false;
    return status;
}
/** Finalize the still-owned destination after success/preparation failure; all resource credits must be exhausted before pool disposal. */
static void finish_reconstruction(void) {
    AmaStatus status = ama_object_destroy(&world, destination);
    assert(status.code == AMA_OK && world.construction_work == NULL);
    destroy_objects();
}
