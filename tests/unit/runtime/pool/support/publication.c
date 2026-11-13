/** @file
 * @brief Check final nominal identity and lifecycle/public visibility gates of reserved pool storage.
 */
/** Reserved identities are final but unavailable through every external access mode until successful lifecycle publication. */
static void it_keeps_final_identity_private_until_lifecycle_publication(void) {
    setup_test();
    AmaPool pool, other;
    AmaStatus status = ama_pool_init(&pool, sources.allocator, 7, sizeof(size_t), 2, UINT64_MAX);
    assert(status.code == AMA_OK);
    status = ama_pool_init(&other, sources.allocator, 8, sizeof(size_t), 2, UINT64_MAX);
    assert(status.code == AMA_OK && allocation_count == 0);
    AmaHandle handle;
    AmaPoolCell *cell = NULL;
    status = ama_pool_reserve(&pool, &handle, &cell);
    assert(status.code == AMA_OK && handle.type_id == 7 && handle.generation != 0);
    assert(cell->metadata.identity.slot_id == handle.slot_id && cell->metadata.identity.generation == handle.generation);
    size_t *address = ama_pool_payload(&pool, cell);
    *address = 91;
    AmaPoolCell sentinel;
    AmaPoolCell *borrow = &sentinel;
    status = ama_pool_lookup(&pool, handle, false, &borrow);
    assert(status.code == AMA_STALE_REFERENCE && borrow == &sentinel);
    status = ama_pool_lookup(&other, handle, true, &borrow);
    assert(status.code == AMA_TYPE_MISMATCH && borrow == &sentinel && other.block_count == 0);
    status = ama_pool_publish(&pool, handle);
    assert(status.code == AMA_INVALID_STATE && pool.live_count == 0);
    status = ama_lifetime_begin(&cell->metadata.lifetime, 1, AMA_CONSTRUCT_DEFAULT);
    assert(status.code == AMA_OK);
    for (AmaAccessMode mode = AMA_ACCESS_ENTRY; mode <= AMA_ACCESS_READ; mode++) {
        status = ama_instance_access(&cell->metadata, mode);
        assert(status.code == AMA_STALE_REFERENCE);
    }
    status = ama_pool_publish(&pool, handle);
    assert(status.code == AMA_INVALID_STATE && pool.live_count == 0);
    status = ama_lifetime_child_ready(&cell->metadata.lifetime);
    assert(status.code == AMA_OK);
    status = ama_lifetime_finish(&cell->metadata.lifetime, false);
    assert(status.code == AMA_OK);
    status = ama_pool_publish(&pool, handle);
    assert(status.code == AMA_OK && pool.live_count == 1);
    status = ama_pool_lookup(&pool, handle, false, &borrow);
    assert(status.code == AMA_OK && borrow == cell && ama_pool_payload(&pool, borrow) == address && *address == 91);
    status = ama_pool_publish(&pool, handle);
    assert(status.code == AMA_INVALID_STATE && pool.live_count == 1);
    status = ama_pool_release(&pool, handle);
    assert(status.code == AMA_INVALID_STATE && pool.live_count == 1);
    status = ama_pool_invalidate(&pool, handle);
    assert(status.code == AMA_OK);
    status = ama_lifetime_begin_cleanup(&cell->metadata.lifetime);
    assert(status.code == AMA_OK);
    size_t child = SIZE_MAX;
    bool next = ama_lifetime_next_child(&cell->metadata.lifetime, &child);
    assert(next && child == 0);
    status = ama_lifetime_end_cleanup(&cell->metadata.lifetime);
    assert(status.code == AMA_OK);
    status = ama_pool_release(&pool, handle);
    assert(status.code == AMA_OK && pool.live_count == 0);
    ama_pool_dispose(&other);
    ama_pool_dispose(&pool);
    destroy_test();
}
