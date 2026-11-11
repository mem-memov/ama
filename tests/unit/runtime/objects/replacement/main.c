#include "../common.c"

static void it_prepares_in_place_without_moving_native_resources_and_preserves_root_identity(void) {
    setup_objects();
    AmaHandle source, destination;
    AmaStatus status = ama_object_create(&world, 2, NULL, &source);
    assert(status.code == AMA_OK);
    status = ama_object_create(&world, 2, NULL, &destination);
    assert(status.code == AMA_OK);
    RootValue *original = object_payload(source);
    original->self = source;
    original->selected = original->left;
    ((LeafValue *)object_payload(original->left))->root = source;
    for (size_t iteration = 0; iteration < 3; iteration++) {
        AmaHandle old_child = ((RootValue *)object_payload(destination))->left;
        status = ama_object_replace(&world, destination, source, false);
        assert(status.code == AMA_OK);
        RootValue *result = object_payload(destination);
        assert(result->native.address == &result->native && result->native.resource != original->native.resource);
        assert(result->self.slot_id == destination.slot_id && result->self.generation == destination.generation);
        assert(result->selected.slot_id == result->left.slot_id);
        assert(((LeafValue *)object_payload(result->left))->root.slot_id == destination.slot_id);
        AmaPoolCell *child = NULL;
        status = ama_pool_lookup(&leaf_type.pool, old_child, false, &child);
        assert(status.code == AMA_STALE_REFERENCE);
        status = ama_pool_lookup(&leaf_type.pool, result->left, false, &child);
        assert(status.code == AMA_OK && child->owner.slot_id == destination.slot_id);
    }
    status = ama_object_destroy(&world, destination);
    assert(status.code == AMA_OK);
    status = ama_object_destroy(&world, source);
    assert(status.code == AMA_OK && constructed == destroyed);
    destroy_objects();
}

static void it_preserves_old_destination_on_every_preparation_allocation_failure(void) {
    size_t count = 0;
    for (size_t failure = 0; failure <= count; failure++) {
        setup_objects();
        AmaHandle source, destination;
        AmaStatus status = ama_object_create(&world, 2, NULL, &source);
        assert(status.code == AMA_OK);
        status = ama_object_create(&world, 2, NULL, &destination);
        assert(status.code == AMA_OK);
        RootValue *old = object_payload(destination);
        void *resource = old->native.resource;
        AmaHandle child = old->left;
        size_t start = allocation_count;
        failure_at = failure == 0 ? 0 : start + failure;
        status = ama_object_replace(&world, destination, source, false);
        if (failure == 0) {
            assert(status.code == AMA_OK);
            count = allocation_count - start;
        } else {
            assert(status.code == AMA_OUT_OF_MEMORY && !world.failed);
            RootValue *unchanged = object_payload(destination);
            assert(unchanged == old && unchanged->native.resource == resource);
            assert(unchanged->left.slot_id == child.slot_id && unchanged->left.generation == child.generation);
            assert(root_type.pool.live_count == 2 && leaf_type.pool.live_count == 4);
        }
        status = ama_object_destroy(&world, destination);
        assert(status.code == AMA_OK);
        status = ama_object_destroy(&world, source);
        assert(status.code == AMA_OK);
        destroy_objects();
    }
}

static void it_cleans_old_and_prepared_resources_once_when_destructive_commit_fails(void) {
    for (size_t failure = 1; failure <= 3; failure++) {
        setup_objects();
        AmaHandle source, destination;
        AmaStatus status = ama_object_create(&world, 2, NULL, &source);
        assert(status.code == AMA_OK);
        status = ama_object_create(&world, 2, NULL, &destination);
        assert(status.code == AMA_OK);
        failed_destructor = failure;
        status = ama_object_replace(&world, destination, source, false);
        assert(status.code == AMA_IO_FAILURE && world.failed && destroyed == 6 && reports == 1);
        AmaPoolCell *cell = NULL;
        status = ama_pool_lookup(&root_type.pool, destination, false, &cell);
        assert(status.code == AMA_STALE_REFERENCE);
        status = ama_object_destroy(&world, destination);
        assert(status.code == AMA_OK && destroyed == 6);
        status = ama_object_destroy(&world, source);
        assert(status.code == AMA_OK && destroyed == 9);
        destroy_objects();
    }
}

static void it_preserves_destination_when_the_prepared_native_constructor_fails(void) {
    setup_objects();
    AmaHandle source, destination;
    AmaStatus status = ama_object_create(&world, 2, NULL, &source);
    assert(status.code == AMA_OK);
    status = ama_object_create(&world, 2, NULL, &destination);
    assert(status.code == AMA_OK);
    RootValue *original = object_payload(destination);
    void *resource = original->native.resource;
    failed_constructor = constructed + 3;
    status = ama_object_replace(&world, destination, source, false);
    assert(status.code == AMA_INVALID_LITERAL && destroyed == 2);
    assert(original == object_payload(destination) && original->native.resource == resource);
    status = ama_object_destroy(&world, destination);
    assert(status.code == AMA_OK);
    status = ama_object_destroy(&world, source);
    assert(status.code == AMA_OK);
    destroy_objects();
}

void run(void) {
    it_prepares_in_place_without_moving_native_resources_and_preserves_root_identity();
    it_preserves_old_destination_on_every_preparation_allocation_failure();
    it_cleans_old_and_prepared_resources_once_when_destructive_commit_fails();
    it_preserves_destination_when_the_prepared_native_constructor_fails();
}

int main(void) { run(); return EXIT_SUCCESS; }
