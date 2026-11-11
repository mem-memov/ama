#include "../objects/common.c"
#include "runtime/lists/list.h"

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
static void it_preserves_order_and_identity_and_invalidates_only_removed_owners(void) {
    setup_objects();
    AmaHandle owner;
    AmaStatus status = ama_object_create(&world, 2, NULL, &owner);
    assert(status.code == AMA_OK);
    RootValue *root = object_payload(owner);
    AmaListRequest request = {.action = AMA_LIST_APPEND};
    for (size_t index = 0; index < 12; index++) {
        status = ama_list_apply(&world, owner, &root->branches, 2, true, &request);
        assert(status.code == AMA_OK && !root->branches.refused);
    }
    AmaHandle removed = root->branches.items[0], survivor = root->branches.items[1];
    request = (AmaListRequest){.action = AMA_LIST_REMOVE};
    status = ama_list_apply(&world, owner, &root->branches, 2, true, &request);
    assert(status.code == AMA_OK && root->branches.count == 11);
    assert(ama_handle_equal(root->branches.items[0], survivor));
    AmaPoolCell *cell = NULL;
    status = ama_pool_lookup(&root_type.pool, removed, false, &cell);
    assert(status.code == AMA_STALE_REFERENCE);
    status = ama_pool_lookup(&root_type.pool, survivor, false, &cell);
    assert(status.code == AMA_OK);
    request = (AmaListRequest){.action = AMA_LIST_SWAP, .secondary = 10};
    status = ama_list_apply(&world, owner, &root->branches, 2, true, &request);
    assert(status.code == AMA_OK && ama_handle_equal(root->branches.items[10], survivor));
    request = (AmaListRequest){.action = AMA_LIST_INSERT, .position = 11};
    status = ama_list_apply(&world, owner, &root->branches, 2, true, &request);
    assert(status.code == AMA_OK && root->branches.count == 12);
    request.position = 13;
    status = ama_list_apply(&world, owner, &root->branches, 2, true, &request);
    assert(status.code == AMA_OK && root->branches.refused && root->branches.outcome == AMA_INDEX_BOUNDS && root->branches.count == 12);
    request.action = AMA_LIST_CLEAR;
    status = ama_list_apply(&world, owner, &root->branches, 2, true, &request);
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

static void it_preflights_all_clear_descendants_and_does_not_acquire_reference_targets(void) {
    setup_objects();
    AmaHandle owner;
    AmaStatus status = ama_object_create(&world, 2, NULL, &owner);
    assert(status.code == AMA_OK);
    RootValue *root = object_payload(owner);
    AmaListRequest request = {.action = AMA_LIST_APPEND};
    for (size_t index = 0; index < 2; index++) {
        status = ama_list_apply(&world, owner, &root->branches, 2, true, &request);
        assert(status.code == AMA_OK);
    }
    RootValue *nested = object_payload(root->branches.items[1]);
    AmaPoolCell *busy = NULL;
    status = ama_pool_lookup(&leaf_type.pool, nested->left, false, &busy);
    assert(status.code == AMA_OK);
    busy->metadata.service.activity = AMA_INSTANCE_WAITING;
    size_t before = destroyed;
    request.action = AMA_LIST_CLEAR;
    status = ama_list_apply(&world, owner, &root->branches, 2, true, &request);
    assert(status.code == AMA_OK && root->branches.refused && root->branches.outcome == AMA_BUSY);
    assert(destroyed == before && root->branches.count == 2);
    request = (AmaListRequest){.action = AMA_LIST_APPEND, .source = nested->left};
    status = ama_list_apply(&world, owner, &root->references, 1, false, &request);
    assert(status.code == AMA_OK && root->references.count == 1);
    request.source = (AmaHandle){0};
    status = ama_list_apply(&world, owner, &root->references, 1, false, &request);
    assert(status.code == AMA_OK && root->references.items[1].generation == 0);
    request.action = AMA_LIST_CLEAR;
    status = ama_list_apply(&world, owner, &root->references, 1, false, &request);
    assert(status.code == AMA_OK && destroyed == before && busy->metadata.service.activity == AMA_INSTANCE_WAITING);
    busy->metadata.service.activity = AMA_INSTANCE_READY;
    status = ama_list_apply(&world, owner, &root->branches, 2, true, &request);
    assert(status.code == AMA_OK && root->branches.count == 0 && !root->branches.refused);
    size_t released = 0;
    status = ama_pool_reclaim(&root_type.pool, &released);
    assert(status.code == AMA_OK);
    status = ama_object_destroy(&world, owner);
    assert(status.code == AMA_OK);
    destroy_objects();
}

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
        AmaListRequest request = {.action = AMA_LIST_APPEND};
        status = ama_list_apply(&world, owner, &root->branches, 2, true, &request);
        failure_at = 0;
        if (failure == 0) { assert(status.code == AMA_OK); allocations = allocation_count - start; }
        else { assert(status.code == AMA_OUT_OF_MEMORY && root->branches.count == 0 && !root->branches.refused); }
        status = ama_object_destroy(&world, owner);
        assert(status.code == AMA_OK);
        destroy_objects();
    }
}

static void it_treats_destructor_failure_as_terminal_and_completes_clear_cleanup(void) {
    setup_objects();
    AmaHandle owner;
    AmaStatus status = ama_object_create(&world, 2, NULL, &owner);
    assert(status.code == AMA_OK);
    RootValue *root = object_payload(owner);
    AmaListRequest request = {.action = AMA_LIST_APPEND};
    for (size_t index = 0; index < 3; index++) {
        status = ama_list_apply(&world, owner, &root->branches, 2, true, &request);
        assert(status.code == AMA_OK);
    }
    failed_destructor = destroyed + 1;
    request.action = AMA_LIST_CLEAR;
    status = ama_list_apply(&world, owner, &root->branches, 2, true, &request);
    assert(status.code == AMA_IO_FAILURE && world.failed && root->branches.count == 0);
    assert(!root->branches.refused && root_type.pool.live_count == 1);
    status = ama_object_destroy(&world, owner);
    assert(status.code == AMA_OK);
    destroy_objects();
}

void run(void) {
    it_treats_destructor_failure_as_terminal_and_completes_clear_cleanup();
    it_grows_geometrically_without_moving_states_and_preserves_data_on_failure();
    it_preserves_order_and_identity_and_invalidates_only_removed_owners();
    it_preflights_all_clear_descendants_and_does_not_acquire_reference_targets();
    it_preserves_contents_after_every_default_insertion_allocation_failure();
}
int main(void) { run(); return EXIT_SUCCESS; }
