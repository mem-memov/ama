/**
 * @file
 * @brief Verify complete forest transfers, continuation copies and allocation rollback.
 */
#include "common.c"

/** Cross-element references use one complete map; ordinary self-copy allocates nothing. */
static void it_clones_one_forest_and_remaps_references_between_sibling_elements(void) {
    setup_objects();
    AmaHandle source, destination;
    make_source(&source, &destination);
    AmaProgram program = {.world = &world};
    AmaProgramAccess access = {.field = 2};
    AmaProgramValue value = {0};
    AmaStatus status = ama_program_select(&program, source, &access, false, &value);
    assert(status.code == AMA_OK);
    status = ama_program_transfer(&program, destination, &access, &value);
    assert(status.code == AMA_OK);
    RootValue *owner = object_payload(destination);
    assert(owner->branches.count == 2);
    RootValue *first = object_payload(owner->branches.items[0]);
    RootValue *second = object_payload(owner->branches.items[1]);
    assert(first->self.slot_id == owner->branches.items[1].slot_id);
    assert(second->self.slot_id == owner->branches.items[0].slot_id);
    assert(first->selected.slot_id == second->left.slot_id);
    size_t allocations = allocation_count;
    status = ama_program_select(&program, destination, &access, false, &value);
    assert(status.code == AMA_OK);
    status = ama_program_transfer(&program, destination, &access, &value);
    assert(status.code == AMA_OK && allocation_count == allocations);
    status = ama_object_destroy(&world, destination);
    assert(status.code == AMA_OK);
    status = ama_object_destroy(&world, source);
    assert(status.code == AMA_OK && constructed == destroyed);
    destroy_objects();
}

/** Enumerate every measured staging allocation failure and verify unchanged old storage. */
static void it_preserves_the_previous_list_on_every_preparation_allocation_failure(void) {
    size_t count = 0;
    for (size_t failure = 0; failure <= count; failure++) {
        setup_objects();
        AmaHandle source, destination;
        make_source(&source, &destination);
        AmaProgram program = {.world = &world};
        AmaProgramAccess access = {.field = 2};
        AmaProgramValue value = {0};
        AmaStatus status = ama_program_select(&program, source, &access, false, &value);
        assert(status.code == AMA_OK);
        size_t start = allocation_count;
        if (failure != 0) { failure_at = start + failure; }
        status = ama_program_transfer(&program, destination, &access, &value);
        if (failure == 0) { assert(status.code == AMA_OK); count = allocation_count - start; }
        else {
            assert(status.code == AMA_OUT_OF_MEMORY);
            assert(((RootValue *)object_payload(destination))->branches.count == 0);
        }
        failure_at = 0;
        world.failed = true;
        status = ama_object_destroy(&world, destination);
        assert(status.code == AMA_OK);
        status = ama_object_destroy(&world, source);
        assert(status.code == AMA_OK);
        destroy_objects();
    }
}

/** Explicit continuation mode preserves only saved positions across the copied forest. */
static void it_copies_continuations_through_one_owned_list_forest(void) {
    for (size_t continuation = 0; continuation < 2; continuation++) {
        setup_objects();
        AmaHandle source, destination;
        make_source(&source, &destination);
        RootValue *original = object_payload(source);
        AmaHandle branch = original->branches.items[0];
        RootValue *child = object_payload(branch);
        AmaPoolCell *nested = NULL;
        AmaStatus status = ama_pool_lookup(&root_type.pool, branch, false, &nested);
        assert(status.code == AMA_OK);
        nested->metadata.service.activity = AMA_INSTANCE_YIELDED;
        nested->metadata.service.resume = (AmaResumePosition){true, 51};
        status = ama_pool_lookup(&leaf_type.pool, child->left, false, &nested);
        assert(status.code == AMA_OK);
        nested->metadata.service.activity = AMA_INSTANCE_YIELDED;
        nested->metadata.service.resume = (AmaResumePosition){true, 52};
        AmaProgram program = {.world = &world};
        AmaProgramAccess access = {.field = 2};
        AmaProgramValue value = {0};
        status = ama_program_select(&program, source, &access, false, &value);
        assert(status.code == AMA_OK);
        value.continuation = continuation != 0;
        status = ama_program_transfer(&program, destination, &access, &value);
        assert(status.code == AMA_OK);
        RootValue *owner = object_payload(destination);
        RootValue *first = object_payload(owner->branches.items[0]);
        assert(first->self.slot_id == owner->branches.items[1].slot_id);
        status = ama_pool_lookup(&root_type.pool, owner->branches.items[0], false, &nested);
        assert(status.code == AMA_OK && nested->metadata.service.resume.selector == (continuation ? 51 : 0));
        status = ama_pool_lookup(&leaf_type.pool, first->left, false, &nested);
        assert(status.code == AMA_OK && nested->metadata.service.resume.selector == (continuation ? 52 : 0));
        status = ama_object_destroy(&world, destination);
        assert(status.code == AMA_OK);
        status = ama_object_destroy(&world, source);
        assert(status.code == AMA_OK);
        destroy_objects();
    }
}

void run(void) {
    it_copies_continuations_through_one_owned_list_forest();
    it_clones_one_forest_and_remaps_references_between_sibling_elements();
    it_preserves_the_previous_list_on_every_preparation_allocation_failure();
}
int main(void) { run(); return EXIT_SUCCESS; }
