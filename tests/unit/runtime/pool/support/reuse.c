/** @file
 * @brief Verify published same-type payloads and controls survive growth and allocation-free free-slot reuse.
 */
/** Every freed slot is reused before allocation; live payloads, addresses and distinct saved positions stay unchanged. */
static void it_reuses_free_slots_without_allocation_or_live_state_changes(void) {
    const size_t block_sizes[] = {1, 2, 7};
    for (size_t shape = 0; shape < sizeof(block_sizes) / sizeof(*block_sizes); shape++) {
        setup_test();
        AmaPool pool;
        AmaStatus status = ama_pool_init(&pool, sources.allocator, 7, sizeof(size_t), block_sizes[shape], UINT64_MAX);
        assert(status.code == AMA_OK && allocation_count == 0 && pool.live_count == 0 && pool.block_count == 0);
        AmaHandle handles[33];
        size_t *addresses[33];
        for (size_t index = 0; index < 33; index++) {
            AmaPoolCell *cell = NULL;
            status = ama_pool_reserve(&pool, &handles[index], &cell);
            assert(status.code == AMA_OK);
            addresses[index] = ama_pool_payload(&pool, cell);
            *addresses[index] = index + 100;
            publish_pool_cell(&pool, handles[index], cell);
            cell->metadata.service.activity = AMA_INSTANCE_YIELDED;
            cell->metadata.service.current_operation = index + 1;
            cell->metadata.service.resume = (AmaResumePosition){true, index + 1};
        }
        for (size_t index = 1; index < 33; index += 2) { release_pool_cell(&pool, handles[index]); }
        assert(pool.live_count == 17);
        size_t allocations = allocation_count;
        size_t blocks = pool.block_count;
        size_t allocated_blocks = pool.allocated_blocks;
        bool reused[33] = {false};
        failure_at = allocations + 1;
        for (size_t index = 0; index < 16; index++) {
            AmaHandle fresh;
            AmaPoolCell *cell = NULL;
            status = ama_pool_reserve(&pool, &fresh, &cell);
            assert(status.code == AMA_OK && allocation_count == allocations);
            size_t previous = 33;
            for (size_t old = 1; old < 33; old += 2) {
                if (handles[old].slot_id == fresh.slot_id) { previous = old; break; }
            }
            assert(previous < 33 && !reused[previous] && fresh.generation > handles[previous].generation);
            reused[previous] = true;
            assert(!cell->metadata.identity.live && cell->metadata.lifetime.phase == AMA_LIFETIME_FRESH);
            assert(cell->metadata.service.current_operation == 0 && !cell->metadata.service.resume.valid);
            assert(ama_pool_payload(&pool, cell) == addresses[previous] && *addresses[previous] == 0);
            assert(*(size_t *)ama_pool_spare_payload(&pool, cell) == 0);
            *addresses[previous] = previous + 100;
            handles[previous] = fresh;
            publish_pool_cell(&pool, fresh, cell);
        }
        assert(pool.live_count == 33 && pool.block_count == blocks && pool.allocated_blocks == allocated_blocks);
        failure_at = 0;
        for (size_t index = 0; index < 33; index++) {
            AmaPoolCell *cell = NULL;
            status = ama_pool_lookup(&pool, handles[index], false, &cell);
            assert(status.code == AMA_OK && ama_pool_payload(&pool, cell) == addresses[index]);
            assert(*addresses[index] == index + 100);
            if (index % 2 == 0) {
                assert(cell->metadata.service.activity == AMA_INSTANCE_YIELDED);
                assert(cell->metadata.service.current_operation == index + 1);
                assert(cell->metadata.service.resume.valid && cell->metadata.service.resume.selector == index + 1);
            }
            release_pool_cell(&pool, handles[index]);
        }
        assert(pool.live_count == 0);
        ama_pool_dispose(&pool);
        destroy_test();
    }
}
