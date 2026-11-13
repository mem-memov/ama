/** @file
 * @brief Fail each native preparation, allocation prefix and destructive old-value hook around final-identity publication.
 */
/** Failure before commit cannot destroy the old value or publish private children. */
static void it_preserves_the_old_tree_when_each_prepared_native_constructor_fails(void) {
    for (size_t failure = 1; failure <= 3; failure++) {
        setup_reconstruction();
        PreviousTree previous = snapshot_tree();
        size_t before = destroyed;
        failed_constructor = constructed + failure;
        AmaStatus status = reconstruct(NULL);
        assert(status.code == AMA_INVALID_LITERAL && destroyed == before + failure - 1);
        assert_previous_tree(&previous);
        finish_reconstruction();
        assert(constructed == destroyed + 1);
    }
}
/** Every preparation allocation failure keeps old identity/state and clears borrowed construction authority, permitting later successful reconstruction. */
static void it_preserves_the_old_tree_at_every_reconstruction_allocation_failure(void) {
    setup_reconstruction();
    size_t before = allocation_count;
    AmaStatus status = reconstruct(NULL);
    assert(status.code == AMA_OK);
    size_t count = allocation_count - before;
    finish_reconstruction();
    for (size_t failure = 1; failure <= count; failure++) {
        setup_reconstruction();
        PreviousTree previous = snapshot_tree();
        failure_at = allocation_count + failure;
        status = reconstruct(NULL);
        assert(status.code == AMA_OUT_OF_MEMORY);
        assert_previous_tree(&previous);
        failure_at = 0;
        status = reconstruct(NULL);
        assert(status.code == AMA_OK);
        assert_constructed_self_tree(destination);
        finish_reconstruction();
    }
}
/** Once old destruction starts, each hook failure is terminal: all old/prepared resources finish cleanup without rollback or duplicate destructor calls. */
static void it_finishes_old_and_prepared_cleanup_when_each_destructive_commit_hook_fails(void) {
    for (size_t failure = 1; failure <= 3; failure++) {
        setup_reconstruction();
        failed_destructor = failure;
        AmaStatus status = reconstruct(NULL);
        assert(status.code == AMA_IO_FAILURE && world.failed && destroyed == 6 && reports == 1);
        AmaPoolCell *cell = NULL;
        status = ama_pool_lookup(&root_type.pool, destination, false, &cell);
        assert(status.code == AMA_STALE_REFERENCE);
        AmaHandle output = {99, 99, 99};
        status = ama_object_create(&world, 2, NULL, &output);
        assert(status.code == AMA_INVALID_STATE && ama_handle_equal(output, (AmaHandle){99, 99, 99}));
        finish_reconstruction();
        assert(constructed == 6 && destroyed == 6);
    }
}
