/** @file
 * @brief Complete resource-free lifecycle ledgers for direct stable-pool tests.
 */
/** Construct and publish exactly the reserved cell, without invoking any module body or native hook. */
static void publish_pool_cell(AmaPool *pool, AmaHandle handle, AmaPoolCell *cell) {
    AmaStatus status = ama_lifetime_begin(&cell->metadata.lifetime, 0, AMA_CONSTRUCT_DEFAULT);
    assert(status.code == AMA_OK);
    status = ama_lifetime_finish(&cell->metadata.lifetime, false);
    assert(status.code == AMA_OK);
    status = ama_pool_publish(pool, handle);
    assert(status.code == AMA_OK);
}
/** Withdraw identity before consuming its empty lifecycle ledger and returning the slot to the free chain. */
static void release_pool_cell(AmaPool *pool, AmaHandle handle) {
    AmaPoolCell *cell = NULL;
    AmaStatus status = ama_pool_lookup(pool, handle, false, &cell);
    assert(status.code == AMA_OK);
    status = ama_pool_invalidate(pool, handle);
    assert(status.code == AMA_OK);
    status = ama_lifetime_begin_cleanup(&cell->metadata.lifetime);
    assert(status.code == AMA_OK);
    status = ama_lifetime_end_cleanup(&cell->metadata.lifetime);
    assert(status.code == AMA_OK);
    status = ama_pool_release(pool, handle);
    assert(status.code == AMA_OK);
}
