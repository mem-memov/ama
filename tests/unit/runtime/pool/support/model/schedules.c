/** @file
 * @brief Enumerate every bounded operation schedule and verify each prefix against issued-identity invariants.
 */
/** Exhaust all 5^6 schedules, block sizes 1..2 and generation limits 1..3, checking every prefix rather than sampling transitions. */
static void it_matches_every_small_pool_schedule_against_identity_and_occupancy_invariants(void) {
    for (size_t slots = 1; slots <= 2; slots++) {
        for (uint64_t limit = 1; limit <= 3; limit++) {
            for (size_t schedule = 0; schedule < 15625; schedule++) {
                setup_test();
                AmaPool pool;
                AmaStatus status = ama_pool_init(&pool, sources.allocator, 7, sizeof(size_t), slots, limit);
                assert(status.code == AMA_OK);
                PoolModel model = {0};
                size_t choices = schedule;
                for (size_t step = 0; step < 6; step++) {
                    size_t action = choices % 5;
                    choices /= 5;
                    if (action == 0) { model_reserve(&pool, &model); }
                    else if (action == 1) { model_publish(&pool, &model); }
                    else if (action == 2 || action == 3) { model_release(&pool, &model, action == 3); }
                    else { model_reclaim(&pool, &model); }
                    assert_pool_model(&pool, &model);
                }
                for (size_t index = 0; index < model.issue_count; index++) { model_release(&pool, &model, false); }
                model_reclaim(&pool, &model);
                assert_pool_model(&pool, &model);
                ama_pool_dispose(&pool);
                destroy_test();
            }
        }
    }
}
