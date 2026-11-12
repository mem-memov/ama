/**
 * @file
 * @brief Verify handle-vector storage/lifecycle invariants after dispatcher removal.
 *
 * This foundation does not claim typed list methods, memberships or traversal.
 */
#include "../objects/common.c"
#include "runtime/lists/list.h"

/**
 * Assert that the component grows geometrically without moving states and preserves data on
 * failure. The case observes actual production behavior through explicit fixture setup and
 * teardown.
 */
static void it_grows_geometrically_without_moving_states_and_preserves_data_on_failure(void) {
    setup_objects();
    AmaHandleList list = {0};
    for (size_t index = 0; index < 100; index++) {
        AmaStatus status = ama_handle_list_reserve(world.allocator, &list, index + 1);
        assert(status.code == AMA_OK && list.capacity >= index + 1);
        list.items[list.count++] = (AmaHandle){1, index, 1};
        for (size_t previous = 0; previous <= index; previous++) { assert(list.items[previous].slot_id == previous); }
    }
    AmaHandle *previous = list.items;
    size_t capacity = list.capacity;
    failure_at = allocation_count + 1;
    AmaStatus status = ama_handle_list_reserve(world.allocator, &list, capacity + 1);
    assert(status.code == AMA_OUT_OF_MEMORY && list.items == previous && list.capacity == capacity && list.count == 100);
    failure_at = 0;
    status = ama_handle_list_reserve(world.allocator, &list, SIZE_MAX);
    assert(status.code == AMA_LIMIT_EXCEEDED && list.items == previous);
    world.allocator.release(world.allocator.context, list.items);
    destroy_objects();
}
/**
 * Assert that the component preserves order and identity and invalidates only removed owners.
 * The case observes actual production behavior through explicit fixture setup and teardown.
 */
static void it_preserves_order_and_identity_and_invalidates_only_removed_owners(void) {
    setup_objects();
    AmaHandle owner;
    AmaStatus status = ama_object_create(&world, 2, NULL, &owner);
    assert(status.code == AMA_OK);
    RootValue *root = object_payload(owner);
    AmaListStorage storage = {&world, owner, &root->branches, 2, true};
    for (size_t index = 0; index < 12; index++) {
        status = ama_list_storage_insert(&storage, root->branches.count, (AmaHandle){0});
        assert(status.code == AMA_OK);
    }
    AmaHandle removed = root->branches.items[0], survivor = root->branches.items[1];
    status = ama_list_storage_remove(&storage, 0, 1);
    assert(status.code == AMA_OK && root->branches.count == 11);
    assert(ama_handle_equal(root->branches.items[0], survivor));
    AmaPoolCell *cell = NULL;
    status = ama_pool_lookup(&root_type.pool, removed, false, &cell);
    assert(status.code == AMA_STALE_REFERENCE);
    status = ama_pool_lookup(&root_type.pool, survivor, false, &cell);
    assert(status.code == AMA_OK);
    status = ama_list_storage_insert(&storage, 11, (AmaHandle){0});
    assert(status.code == AMA_OK && root->branches.count == 12);
    status = ama_list_storage_insert(&storage, 13, (AmaHandle){0});
    assert(status.code == AMA_INDEX_BOUNDS && root->branches.count == 12);
    status = ama_list_storage_remove(&storage, 1, SIZE_MAX);
    assert(status.code == AMA_INDEX_BOUNDS && root->branches.count == 12);
    status = ama_list_storage_remove(&storage, 0, root->branches.count);
    assert(status.code == AMA_OK && root->branches.count == 0);
    size_t released = 0;
    status = ama_pool_reclaim(&root_type.pool, &released);
    assert(status.code == AMA_OK && released != 0);
    status = ama_pool_lookup(&root_type.pool, survivor, false, &cell);
    assert(status.code == AMA_STALE_REFERENCE);
    status = ama_object_destroy(&world, owner);
    assert(status.code == AMA_OK);
    destroy_objects();
}

/**
 * Assert that the component preflights all clear descendants and does not acquire reference
 * targets. The case observes actual production behavior through explicit fixture setup and
 * teardown.
 */
static void it_preflights_all_clear_descendants_and_does_not_acquire_reference_targets(void) {
    setup_objects();
    AmaHandle owner;
    AmaStatus status = ama_object_create(&world, 2, NULL, &owner);
    assert(status.code == AMA_OK);
    RootValue *root = object_payload(owner);
    AmaListStorage storage = {&world, owner, &root->branches, 2, true};
    for (size_t index = 0; index < 2; index++) {
        status = ama_list_storage_insert(&storage, root->branches.count, (AmaHandle){0});
        assert(status.code == AMA_OK);
    }
    RootValue *nested = object_payload(root->branches.items[1]);
    AmaPoolCell *busy = NULL;
    status = ama_pool_lookup(&leaf_type.pool, nested->left, false, &busy);
    assert(status.code == AMA_OK);
    busy->metadata.service.activity = AMA_INSTANCE_WAITING;
    size_t before = destroyed;
    status = ama_list_storage_remove(&storage, 0, root->branches.count);
    assert(status.code == AMA_BUSY);
    assert(destroyed == before && root->branches.count == 2);
    AmaListStorage references = {&world, owner, &root->references, 1, false};
    status = ama_list_storage_insert(&references, 0, nested->left);
    assert(status.code == AMA_OK && root->references.count == 1);
    status = ama_list_storage_insert(&references, 1, (AmaHandle){0});
    assert(status.code == AMA_OK && root->references.items[1].generation == 0);
    status = ama_list_storage_remove(&references, 0, root->references.count);
    assert(status.code == AMA_OK && destroyed == before && busy->metadata.service.activity == AMA_INSTANCE_WAITING);
    busy->metadata.service.activity = AMA_INSTANCE_READY;
    status = ama_list_storage_remove(&storage, 0, root->branches.count);
    assert(status.code == AMA_OK && root->branches.count == 0);
    size_t released = 0;
    status = ama_pool_reclaim(&root_type.pool, &released);
    assert(status.code == AMA_OK);
    status = ama_object_destroy(&world, owner);
    assert(status.code == AMA_OK);
    destroy_objects();
}

/**
 * Assert that the component preserves contents after every default insertion allocation failure.
 * The case observes actual production behavior through explicit fixture setup and teardown.
 */
static void it_preserves_contents_after_every_default_insertion_allocation_failure(void) {
    size_t allocations = 0;
    for (size_t failure = 0; failure <= allocations; failure++) {
        setup_objects();
        AmaHandle owner;
        AmaStatus status = ama_object_create(&world, 2, NULL, &owner);
        assert(status.code == AMA_OK);
        RootValue *root = object_payload(owner);
        size_t start = allocation_count;
        failure_at = failure == 0 ? 0 : start + failure;
        AmaListStorage storage = {&world, owner, &root->branches, 2, true};
        status = ama_list_storage_insert(&storage, root->branches.count, (AmaHandle){0});
        failure_at = 0;
        if (failure == 0) { assert(status.code == AMA_OK); allocations = allocation_count - start; }
        else { assert(status.code == AMA_OUT_OF_MEMORY && root->branches.count == 0); }
        status = ama_object_destroy(&world, owner);
        assert(status.code == AMA_OK);
        destroy_objects();
    }
}

/**
 * Assert that the component treats destructor failure as terminal and completes clear cleanup.
 * The case observes actual production behavior through explicit fixture setup and teardown.
 */
static void it_treats_destructor_failure_as_terminal_and_completes_clear_cleanup(void) {
    setup_objects();
    AmaHandle owner;
    AmaStatus status = ama_object_create(&world, 2, NULL, &owner);
    assert(status.code == AMA_OK);
    RootValue *root = object_payload(owner);
    AmaListStorage storage = {&world, owner, &root->branches, 2, true};
    for (size_t index = 0; index < 3; index++) {
        status = ama_list_storage_insert(&storage, root->branches.count, (AmaHandle){0});
        assert(status.code == AMA_OK);
    }
    failed_destructor = destroyed + 1;
    status = ama_list_storage_remove(&storage, 0, root->branches.count);
    assert(status.code == AMA_IO_FAILURE && world.failed && root->branches.count == 0);
    assert(root_type.pool.live_count == 1);
    status = ama_object_destroy(&world, owner);
    assert(status.code == AMA_OK);
    destroy_objects();
}

/** The retained construction primitive clones state without copying resumability. */
static void it_clones_inserted_owned_state_and_remaps_internal_references(void) {
    setup_objects();
    AmaHandle owner, source;
    AmaStatus status = ama_object_create(&world, 2, NULL, &owner);
    assert(status.code == AMA_OK);
    status = ama_object_create(&world, 2, NULL, &source);
    assert(status.code == AMA_OK);
    RootValue *original = object_payload(source);
    original->self = source;
    original->selected = original->left;
    AmaPoolCell *source_cell = NULL;
    status = ama_pool_lookup(&root_type.pool, source, false, &source_cell);
    assert(status.code == AMA_OK);
    source_cell->metadata.service.activity = AMA_INSTANCE_YIELDED;
    source_cell->metadata.service.resume = (AmaResumePosition){true, 9};
    RootValue *root = object_payload(owner);
    AmaListStorage storage = {&world, owner, &root->branches, 2, true};
    status = ama_list_storage_insert(&storage, 0, source);
    assert(status.code == AMA_OK && root->branches.count == 1);
    AmaHandle handle = root->branches.items[0];
    RootValue *copy = object_payload(handle);
    assert(!ama_handle_equal(handle, source));
    assert(ama_handle_equal(copy->self, handle));
    assert(ama_handle_equal(copy->selected, copy->left));
    assert(copy->native.number == original->native.number);
    assert(copy->native.resource != original->native.resource);
    AmaPoolCell *copy_cell = NULL;
    status = ama_pool_lookup(&root_type.pool, handle, false, &copy_cell);
    assert(status.code == AMA_OK && !copy_cell->metadata.service.resume.valid);
    assert(source_cell->metadata.service.resume.selector == 9);
    status = ama_object_destroy(&world, owner);
    assert(status.code == AMA_OK);
    status = ama_object_destroy(&world, source);
    assert(status.code == AMA_OK);
    destroy_objects();
}

/** Native lifecycle reentry cannot start another vector mutation. */
static void it_rejects_reentrant_storage_work_without_mutating_or_allocating(void) {
    setup_objects();
    AmaHandle owner;
    AmaStatus status = ama_object_create(&world, 2, NULL, &owner);
    assert(status.code == AMA_OK);
    RootValue *root = object_payload(owner);
    AmaListStorage storage = {&world, owner, &root->branches, 2, true};
    size_t allocations = allocation_count;
    world.operation_active = true;
    status = ama_list_storage_insert(&storage, 0, (AmaHandle){0});
    assert(status.code == AMA_INVALID_STATE);
    status = ama_list_storage_remove(&storage, 0, 0);
    assert(status.code == AMA_INVALID_STATE);
    world.operation_active = false;
    world.failed = true;
    status = ama_list_storage_insert(&storage, 0, (AmaHandle){0});
    assert(status.code == AMA_INVALID_STATE);
    assert(allocation_count == allocations && root->branches.count == 0);
    world.failed = false;
    status = ama_object_destroy(&world, owner);
    assert(status.code == AMA_OK);
    destroy_objects();
}

/**
 * Execute every named assertion case selected by this leaf; the recursive harness owns process
 * status and success-only cleanup.
 */
void run(void) {
    it_rejects_reentrant_storage_work_without_mutating_or_allocating();
    it_clones_inserted_owned_state_and_remaps_internal_references();
    it_treats_destructor_failure_as_terminal_and_completes_clear_cleanup();
    it_grows_geometrically_without_moving_states_and_preserves_data_on_failure();
    it_preserves_order_and_identity_and_invalidates_only_removed_owners();
    it_preflights_all_clear_descendants_and_does_not_acquire_reference_targets();
    it_preserves_contents_after_every_default_insertion_allocation_failure();
}
/**
 * Run the explicit assertion suite and return success only after every selected case completes;
 * failed assertions preserve generated leaf evidence.
 */
int main(void) { run(); return EXIT_SUCCESS; }
