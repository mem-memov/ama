/** @file
 * @brief Fail native constructors and every allocation prefix after self identity formation, then prove stale identities never revive.
 */
/** A failure at either child or the root releases completed prefixes; observed self identities stay stale across reuse, exhaustion and reclamation. */
static void it_never_revives_self_handles_observed_before_any_failed_constructor(void) {
    for (size_t failure = 1; failure <= 3; failure++) {
        setup_observed_construction();
        failed_constructor = failure;
        AmaHandle root = {99, 99, 99};
        AmaStatus status = ama_object_create(&world, 2, NULL, &root);
        assert(status.code == AMA_INVALID_LITERAL && ama_handle_equal(root, (AmaHandle){99, 99, 99}));
        assert(!world.failed && !world.operation_active && constructed == failure && destroyed == failure - 1);
        assert(issued_count >= 2 && issued_count <= 3 && root_type.pool.live_count == 0 && leaf_type.pool.live_count == 0);
        AmaHandle failed[3];
        size_t count = issued_count;
        memcpy(failed, issued, count * sizeof(*failed));
        assert_failed_identities_stale(failed, count);
        for (size_t retry = 0; retry < 8; retry++) {
            status = ama_object_create(&world, 2, NULL, &root);
            assert(status.code == AMA_OK);
            assert_constructed_self_tree(root);
            assert_failed_identities_stale(failed, count);
            status = ama_object_destroy(&world, root);
            assert(status.code == AMA_OK);
            if (retry % 2 == 1) {
                size_t released;
                status = ama_pool_reclaim(&root_type.pool, &released);
                assert(status.code == AMA_OK);
                status = ama_pool_reclaim(&leaf_type.pool, &released);
                assert(status.code == AMA_OK);
            }
            assert_failed_identities_stale(failed, count);
        }
        assert(constructed == destroyed + 1);
        destroy_objects();
    }
}
/** All source-recipe, reservation and native-resource allocation failures preserve output, release private state and allow a clean fresh retry. */
static void it_releases_every_allocation_prefix_of_self_initialization(void) {
    setup_observed_construction();
    size_t before = allocation_count;
    AmaHandle root;
    AmaStatus status = ama_object_create(&world, 2, NULL, &root);
    assert(status.code == AMA_OK);
    size_t count = allocation_count - before;
    status = ama_object_destroy(&world, root);
    assert(status.code == AMA_OK);
    destroy_objects();
    bool observed_private = false;
    for (size_t failure = 1; failure <= count; failure++) {
        setup_observed_construction();
        failure_at = allocation_count + failure;
        root = (AmaHandle){99, 99, 99};
        status = ama_object_create(&world, 2, NULL, &root);
        assert(status.code == AMA_OUT_OF_MEMORY && ama_handle_equal(root, (AmaHandle){99, 99, 99}));
        assert(!world.failed && !world.operation_active && root_type.pool.live_count == 0 && leaf_type.pool.live_count == 0);
        AmaHandle failed[3];
        size_t failed_count = issued_count;
        assert(failed_count <= 3);
        memcpy(failed, issued, failed_count * sizeof(*failed));
        observed_private |= failed_count != 0;
        assert_failed_identities_stale(failed, failed_count);
        failure_at = 0;
        status = ama_object_create(&world, 2, NULL, &root);
        assert(status.code == AMA_OK);
        assert_constructed_self_tree(root);
        assert_failed_identities_stale(failed, failed_count);
        status = ama_object_destroy(&world, root);
        assert(status.code == AMA_OK);
        destroy_objects();
    }
    assert(observed_private);
}
