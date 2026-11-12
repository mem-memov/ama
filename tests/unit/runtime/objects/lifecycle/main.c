/**
 * @file
 * @brief Check selected constructors, child/native cleanup order and every failed initialization prefix.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../../docs/implementation/library-tests-and-build.md)
 */
#include "../common.c"

/**
 * Assert that the component constructs children before payload and finalizes parent before
 * reverse children. The explicit fixture setup and observations define this regression;
 * assertion failure keeps its generated evidence.
 */
static void it_constructs_children_before_payload_and_finalizes_parent_before_reverse_children(void) {
    setup_objects();
    AmaHandle root;
    AmaStatus status = ama_object_create(&world, 2, NULL, &root);
    assert(status.code == AMA_OK && constructed == 3 && leaf_type.pool.live_count == 2);
    status = ama_object_destroy(&world, root);
    assert(status.code == AMA_OK && destroyed == 3);
    assert(destruction_order[0] == 3 && destruction_order[1] == 2 && destruction_order[2] == 1);
    status = ama_object_destroy(&world, root);
    assert(status.code == AMA_STALE_REFERENCE && destroyed == 3);
    destroy_objects();
}

/**
 * Assert that the component uses one selected constructor without default then overwrite. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_uses_one_selected_constructor_without_default_then_overwrite(void) {
    setup_objects();
    const unsigned char byte = 42;
    AmaRecipe child = {.constructor = AMA_CONSTRUCT_LITERAL, .literal = {&byte, 1}};
    AmaFieldRecipe fields[6] = {{.children = &child}};
    AmaRecipe recipe = {.fields = fields, .field_count = 6};
    AmaHandle root;
    AmaStatus status = ama_object_create(&world, 2, &recipe, &root);
    assert(status.code == AMA_OK && constructed == 3);
    RootValue *value = object_payload(root);
    assert(((LeafValue *)object_payload(value->left))->native.number == 42);
    status = ama_object_destroy(&world, root);
    assert(status.code == AMA_OK && destroyed == 3);
    destroy_objects();
}

/**
 * Assert that the component cleans every failed prefix without destroying the failed native
 * payload. The explicit fixture setup and observations define this regression; assertion failure
 * keeps its generated evidence.
 */
static void it_cleans_every_failed_prefix_without_destroying_the_failed_native_payload(void) {
    for (size_t failure = 1; failure <= 3; failure++) {
        setup_objects();
        failed_constructor = failure;
        AmaHandle root = {0};
        AmaStatus status = ama_object_create(&world, 2, NULL, &root);
        assert(status.code == AMA_INVALID_LITERAL && root.generation == 0);
        assert(destroyed == failure - 1 && reports == 1);
        destroy_objects();
    }
}

/**
 * Assert that the component continues cleanup after errors and marks the world terminal. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_continues_cleanup_after_errors_and_marks_the_world_terminal(void) {
    setup_objects();
    AmaHandle root;
    AmaStatus status = ama_object_create(&world, 2, NULL, &root);
    assert(status.code == AMA_OK);
    failed_destructor = 1;
    status = ama_object_destroy(&world, root);
    assert(status.code == AMA_IO_FAILURE && destroyed == 3 && reports == 1 && world.failed);
    status = ama_object_create(&world, 2, NULL, &root);
    assert(status.code == AMA_INVALID_STATE);
    destroy_objects();
}

/**
 * Assert that the component unwinds every construction allocation failure. The explicit fixture
 * setup and observations define this regression; assertion failure keeps its generated evidence.
 */
static void it_unwinds_every_construction_allocation_failure(void) {
    setup_objects();
    AmaHandle root;
    AmaStatus status = ama_object_create(&world, 2, NULL, &root);
    assert(status.code == AMA_OK);
    size_t count = allocation_count;
    status = ama_object_destroy(&world, root);
    assert(status.code == AMA_OK);
    destroy_objects();
    for (size_t failure = 1; failure <= count; failure++) {
        setup_objects();
        failure_at = failure;
        root = (AmaHandle){0};
        status = ama_object_create(&world, 2, NULL, &root);
        assert(status.code == AMA_OUT_OF_MEMORY && root.generation == 0);
        destroy_objects();
    }
}

/**
 * Assert that the component preserves the construction error when secondary cleanup also fails.
 * The explicit fixture setup and observations define this regression; assertion failure keeps
 * its generated evidence.
 */
static void it_preserves_the_construction_error_when_secondary_cleanup_also_fails(void) {
    setup_objects();
    failed_constructor = 3;
    failed_destructor = 1;
    AmaHandle root = {0};
    AmaStatus status = ama_object_create(&world, 2, NULL, &root);
    assert(status.code == AMA_INVALID_LITERAL && reports == 2 && world.failed);
    assert(destroyed == 2);
    destroy_objects();
}

/**
 * Assert that the component unwinds list buffers and nested ownership on each allocation
 * failure. The explicit fixture setup and observations define this regression; assertion failure
 * keeps its generated evidence.
 */
static void it_unwinds_list_buffers_and_nested_ownership_on_each_allocation_failure(void) {
    size_t count = 0;
    for (size_t failure = 0; failure <= count; failure++) {
        setup_objects();
        AmaFieldRecipe fields[6] = {0};
        fields[2].count = 2;
        fields[5].count = 3;
        AmaRecipe recipe = {.fields = fields, .field_count = 6};
        failure_at = failure;
        AmaHandle root = {0};
        AmaStatus status = ama_object_create(&world, 2, &recipe, &root);
        if (failure == 0) {
            assert(status.code == AMA_OK && constructed == 9);
            count = allocation_count;
            status = ama_object_destroy(&world, root);
            assert(status.code == AMA_OK && destroyed == 9);
        } else {
            assert(status.code == AMA_OUT_OF_MEMORY && root.generation == 0);
        }
        destroy_objects();
    }
}

/**
 * Run every explicitly selected assertion case in this executable; a failed assertion preserves
 * leaf generated output for diagnosis.
 */
void run(void) {
    it_constructs_children_before_payload_and_finalizes_parent_before_reverse_children();
    it_uses_one_selected_constructor_without_default_then_overwrite();
    it_cleans_every_failed_prefix_without_destroying_the_failed_native_payload();
    it_continues_cleanup_after_errors_and_marks_the_world_terminal();
    it_unwinds_every_construction_allocation_failure();
    it_preserves_the_construction_error_when_secondary_cleanup_also_fails();
    it_unwinds_list_buffers_and_nested_ownership_on_each_allocation_failure();
}

/**
 * Enter the explicit test run and return success only if all assertions complete; the parent
 * harness observes this process status.
 */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
