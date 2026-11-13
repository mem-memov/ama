/**
 * @file
 * @brief Observe real pool invalidation, child lifetime and blocked destruction reentry from native cleanup.
 */
static AmaHandle observed_lifecycle_root;
static size_t observed_lifecycle_hooks;

/** Inspect runtime-owned credits without retaining any Ama address beyond this test callback.
 * The parent credit is consumed before entry while both owned children remain live.
 * The public destruction guard rejects reentry before any second cleanup starts.
 */
static AmaStatus observe_lifecycle_destroy(void *address) {
    NativeValue *native = address;
    AmaPoolCell *root_cell = NULL;
    assert(ama_pool_lookup(&root_type.pool, observed_lifecycle_root, true, &root_cell).code == AMA_OK);
    RootValue *root = ama_pool_payload(&root_type.pool, root_cell);
    AmaPoolCell *inaccessible = NULL;
    assert(ama_pool_lookup(&root_type.pool, observed_lifecycle_root, false, &inaccessible).code == AMA_STALE_REFERENCE);
    assert(root_cell->metadata.lifetime.phase == AMA_LIFETIME_FINALIZED);
    assert(!root_cell->metadata.lifetime.native_live && root_cell->metadata.lifetime.cleanup_active);
    assert(ama_object_destroy(&world, observed_lifecycle_root).code == AMA_BUSY);
    if (address == &root->native) {
        AmaHandle children[] = {root->left,root->right};
        for (size_t index = 0; index < 2; index++) {
            AmaPoolCell *child = NULL;
            assert(ama_pool_lookup(&leaf_type.pool, children[index], false, &child).code == AMA_OK);
            assert(child->metadata.lifetime.phase == AMA_LIFETIME_LIVE && child->metadata.lifetime.native_live);
            LeafValue *payload = ama_pool_payload(&leaf_type.pool, child);
            assert(payload->native.resource != NULL);
        }
        assert(observed_lifecycle_hooks == 0);
    } else {
        AmaHandle handle = observed_lifecycle_hooks == 1 ? root->right : root->left;
        AmaPoolCell *child = NULL;
        assert(ama_pool_lookup(&leaf_type.pool, handle, false, &child).code == AMA_STALE_REFERENCE);
        assert(ama_pool_lookup(&leaf_type.pool, handle, true, &child).code == AMA_OK);
        assert(child->metadata.lifetime.phase == AMA_LIFETIME_FINALIZED && !child->metadata.lifetime.native_live);
        assert((unsigned char *)address == (unsigned char *)ama_pool_payload(&leaf_type.pool, child) + leaf_type.native_offset);
    }
    assert(native->resource != NULL);
    observed_lifecycle_hooks++;
    return destroy_payload(address);
}

/** All three destructor failure positions invalidate first, finish the whole tree and never retry. */
static void it_invalidates_before_native_cleanup_and_preserves_live_owned_children(void) {
    for (size_t failed = 0; failed <= 3; failed++) {
        setup_objects();
        observed_lifecycle_root = (AmaHandle){0};
        observed_lifecycle_hooks = 0;
        root_type.hooks.destroy = observe_lifecycle_destroy;
        leaf_type.hooks.destroy = observe_lifecycle_destroy;
        assert(ama_object_create(&world, 2, NULL, &observed_lifecycle_root).code == AMA_OK);
        failed_destructor = failed;
        AmaStatus status = ama_object_destroy(&world, observed_lifecycle_root);
        assert(status.code == (failed == 0 ? AMA_OK : AMA_IO_FAILURE));
        assert(observed_lifecycle_hooks == 3 && destroyed == 3 && reports == (failed == 0 ? 0 : 1));
        assert(destruction_order[0] == 3 && destruction_order[1] == 2 && destruction_order[2] == 1);
        assert(ama_object_destroy(&world, observed_lifecycle_root).code == AMA_STALE_REFERENCE);
        assert(observed_lifecycle_hooks == 3 && destroyed == 3);
        destroy_objects();
    }
}
