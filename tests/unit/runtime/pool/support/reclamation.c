/** @file
 * @brief Exercise interior reclamation, absent directories, retained free links and failed regrowth with live later blocks.
 */
/** Sparse holes permanently retire old IDs, retain partial-block capacity and never move later live or constructing payloads. */
static void it_reclaims_sparse_blocks_without_renumbering_or_reviving_history(void) {
    setup_test();
    AmaPool pool;
    AmaStatus status = ama_pool_init(&pool, sources.allocator, 7, sizeof(size_t), 2, 3);
    assert(status.code == AMA_OK);
    AmaHandle handles[8];
    size_t *addresses[8];
    for (size_t index = 0; index < 8; index++) {
        AmaPoolCell *cell = NULL;
        status = ama_pool_reserve(&pool, &handles[index], &cell);
        assert(status.code == AMA_OK);
        addresses[index] = ama_pool_payload(&pool, cell);
        *addresses[index] = index + 40;
        if (index != 6) { publish_pool_cell(&pool, handles[index], cell); }
    }
    /* Blocks zero and two become holes; block one retains a free reusable slot,
     * and block three retains a private construction reservation plus a live peer. */
    const size_t removed[] = {0, 1, 2, 4, 5};
    for (size_t index = 0; index < sizeof(removed) / sizeof(*removed); index++) {
        release_pool_cell(&pool, handles[removed[index]]);
    }
    assert(pool.live_count == 2);
    size_t allocations = allocation_count;
    failure_at = allocations + 1;
    size_t released = SIZE_MAX;
    status = ama_pool_reclaim(&pool, &released);
    assert(status.code == AMA_OK && released == 2 && allocation_count == allocations);
    assert(pool.block_count == 4 && pool.allocated_blocks == 2 && pool.live_count == 2);
    assert(pool.blocks[0].memory == NULL && pool.blocks[2].memory == NULL);
    assert(pool.blocks[1].occupied == 1 && pool.blocks[3].occupied == 2);
    for (size_t index = 0; index < 8; index++) {
        AmaPoolCell sentinel;
        AmaPoolCell *cell = &sentinel;
        bool retained = index == 3 || index == 6 || index == 7;
        status = ama_pool_lookup(&pool, handles[index], true, &cell);
        if (retained) {
            assert(status.code == AMA_OK && ama_pool_payload(&pool, cell) == addresses[index]);
            assert(*addresses[index] == index + 40);
        } else { assert(status.code == AMA_STALE_REFERENCE && cell == &sentinel); }
    }
    AmaHandle reused;
    AmaPoolCell *cell = NULL;
    status = ama_pool_reserve(&pool, &reused, &cell);
    assert(status.code == AMA_OK && reused.slot_id == handles[2].slot_id && allocation_count == allocations);
    publish_pool_cell(&pool, reused, cell);
    AmaHandle fresh = {99, 99, 99};
    AmaPoolCell *previous = cell;
    for (size_t failure = 1; failure <= 2; failure++) {
        failure_at = allocation_count + failure;
        status = ama_pool_reserve(&pool, &fresh, &cell);
        assert(status.code == AMA_OUT_OF_MEMORY && cell == previous && ama_handle_equal(fresh, (AmaHandle){99, 99, 99}));
        assert(pool.block_count == 4 && pool.allocated_blocks == 2);
    }
    failure_at = 0;
    status = ama_pool_reserve(&pool, &fresh, &cell);
    assert(status.code == AMA_OK && fresh.slot_id >= 8 && pool.block_count == 5 && pool.allocated_blocks == 3);
    publish_pool_cell(&pool, fresh, cell);
    for (size_t index = 0; index < sizeof(removed) / sizeof(*removed); index++) {
        status = ama_pool_lookup(&pool, handles[removed[index]], true, &cell);
        assert(status.code == AMA_STALE_REFERENCE);
    }
    status = ama_pool_lookup(&pool, handles[6], true, &cell);
    assert(status.code == AMA_OK && ama_pool_payload(&pool, cell) == addresses[6]);
    publish_pool_cell(&pool, handles[6], cell);
    release_pool_cell(&pool, handles[3]);
    release_pool_cell(&pool, handles[6]);
    release_pool_cell(&pool, handles[7]);
    release_pool_cell(&pool, reused);
    release_pool_cell(&pool, fresh);
    status = ama_pool_reclaim(&pool, &released);
    assert(status.code == AMA_OK && released == 3 && pool.allocated_blocks == 0 && pool.free_head == SIZE_MAX);
    status = ama_pool_reclaim(&pool, &released);
    assert(status.code == AMA_OK && released == 0);
    ama_pool_dispose(&pool);
    destroy_test();
}
