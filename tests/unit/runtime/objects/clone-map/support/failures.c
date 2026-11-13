/** @file
 * @brief Verify source/native ownership and all allocation prefixes of scalar clone preparation.
 */
/** Seed self/peer/ancestor/duplicate external references for failed-preparation cases as well as successful copies. */
static void seed_failed_map(void) {
    RootValue *root = object_payload(source_root);
    LeafValue *left = object_payload(root->left), *right = object_payload(root->right);
    root->self = source_root;
    root->selected = external_leaf;
    left->peer = right->peer = external_leaf;
    left->root = source_root;
    right->root = external_root;
}
/** No failed native clone publishes a partial root or mutates its source; exactly the successful native prefix is destroyed. */
static void it_preserves_source_and_external_state_when_each_native_clone_fails(void) {
    for (size_t failure = 1; failure <= 3; failure++) {
        setup_map();
        seed_failed_map();
        size_t before = destroyed;
        failed_constructor = constructed + failure;
        AmaHandle copy = {99, 99, 99};
        AmaStatus status = ama_object_clone(&world, source_root, false, &copy);
        assert(status.code == AMA_INVALID_LITERAL && ama_handle_equal(copy, (AmaHandle){99, 99, 99}));
        assert(cloned == failure && destroyed == before + failure - 1);
        assert(root_type.pool.live_count == 2 && leaf_type.pool.live_count == 5);
        RootValue *root = object_payload(source_root);
        assert(ama_handle_equal(root->self, source_root) && ama_handle_equal(root->selected, external_leaf));
        assert(ama_handle_equal(((LeafValue *)object_payload(root->left))->root, source_root));
        finish_map();
    }
}
/** Every protection/work/storage/native allocation failure preserves output, both external targets and the exact source tree; scratch prefixes are reclaimed. */
static void it_unwinds_every_scalar_clone_allocation_without_publishing_or_reinitializing(void) {
    setup_map();
    seed_failed_map();
    size_t before = allocation_count;
    AmaHandle copy;
    AmaStatus status = ama_object_clone(&world, source_root, false, &copy);
    assert(status.code == AMA_OK);
    size_t count = allocation_count - before;
    assert_scalar_copy(copy);
    status = ama_object_destroy(&world, copy);
    assert(status.code == AMA_OK);
    finish_map();
    bool native_prefix = false;
    for (size_t failure = 1; failure <= count; failure++) {
        setup_map();
        seed_failed_map();
        failure_at = allocation_count + failure;
        copy = (AmaHandle){99, 99, 99};
        status = ama_object_clone(&world, source_root, false, &copy);
        assert(status.code == AMA_OUT_OF_MEMORY && ama_handle_equal(copy, (AmaHandle){99, 99, 99}));
        native_prefix |= cloned != 0;
        assert(root_type.pool.live_count == 2 && leaf_type.pool.live_count == 5);
        RootValue *root = object_payload(source_root);
        assert(ama_handle_equal(root->self, source_root) && ama_handle_equal(root->selected, external_leaf));
        failure_at = 0;
        finish_map();
    }
    assert(native_prefix);
}
