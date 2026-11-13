/** @file
 * @brief Fail later directory/payload growth while retaining published typed identities and live addresses.
 */
/** Each later growth allocation can fail without publishing outputs, changing existing objects or preventing retry. */
static void it_preserves_live_instances_through_each_later_growth_failure(void) {
    for (size_t failure = 1; failure <= 2; failure++) {
        setup_test();
        AmaPool pool;
        AmaStatus status = ama_pool_init(&pool, sources.allocator, 7, sizeof(size_t), 1, UINT64_MAX);
        assert(status.code == AMA_OK);
        AmaHandle live[2];
        size_t *addresses[2];
        for (size_t index = 0; index < 2; index++) {
            AmaPoolCell *cell = NULL;
            status = ama_pool_reserve(&pool, &live[index], &cell);
            assert(status.code == AMA_OK);
            addresses[index] = ama_pool_payload(&pool, cell);
            *addresses[index] = index + 27;
            publish_pool_cell(&pool, live[index], cell);
        }
        assert(pool.block_count == 2 && pool.directory_capacity == 2 && pool.free_head == SIZE_MAX);
        failure_at = allocation_count + failure;
        AmaHandle fresh = {99, 99, 99};
        AmaPoolCell sentinel;
        AmaPoolCell *cell = &sentinel;
        status = ama_pool_reserve(&pool, &fresh, &cell);
        assert(status.code == AMA_OUT_OF_MEMORY && ama_handle_equal(fresh, (AmaHandle){99, 99, 99}));
        assert(cell == &sentinel && pool.block_count == 2 && pool.allocated_blocks == 2 && pool.live_count == 2);
        assert(pool.free_head == SIZE_MAX);
        for (size_t index = 0; index < 2; index++) {
            status = ama_pool_lookup(&pool, live[index], false, &cell);
            assert(status.code == AMA_OK && ama_pool_payload(&pool, cell) == addresses[index]);
            assert(*addresses[index] == index + 27 && pool.blocks[index].occupied == 1);
        }
        failure_at = 0;
        status = ama_pool_reserve(&pool, &fresh, &cell);
        assert(status.code == AMA_OK && pool.block_count == 3 && pool.live_count == 2);
        publish_pool_cell(&pool, fresh, cell);
        release_pool_cell(&pool, fresh);
        for (size_t index = 0; index < 2; index++) { release_pool_cell(&pool, live[index]); }
        ama_pool_dispose(&pool);
        destroy_test();
    }
}
