/** @file
 * @brief Apply finite model transitions independently of the implementation free-list order.
 */
/** Reserve exactly one new identity, checking reuse-before-growth and generation against independent history. */
static void model_reserve(AmaPool *pool, PoolModel *model) {
    bool free = false;
    for (size_t slot = 0; slot < model->block_count * pool->slots_per_block; slot++) {
        if (model_free(model, slot, pool->slots_per_block, pool->generation_limit)) { free = true; }
    }
    size_t allocations = allocation_count;
    AmaHandle handle;
    AmaPoolCell *cell = NULL;
    AmaStatus status = ama_pool_reserve(pool, &handle, &cell);
    assert(status.code == AMA_OK);
    size_t block = handle.slot_id / pool->slots_per_block;
    if (free) {
        assert(model_free(model, handle.slot_id, pool->slots_per_block, pool->generation_limit));
        assert(allocation_count == allocations);
    } else {
        assert(block == model->block_count && model->block_count < 6);
        model->present[model->block_count++] = true;
    }
    const PoolModelIssue *previous = model_previous(model, handle.slot_id);
    assert(handle.type_id == pool->type_id && handle.generation == (previous == NULL ? 1 : previous->handle.generation + 1));
    size_t index = model->issue_count++;
    assert(index < 6);
    void *address = ama_pool_payload(pool, cell);
    *(size_t *)address = index + 10;
    model->issues[index] = (PoolModelIssue){handle, address, true, false};
}
/** Publish the oldest still-private identity; no eligible identity is an explicit no-op in this finite scheduler. */
static void model_publish(AmaPool *pool, PoolModel *model) {
    for (size_t index = 0; index < model->issue_count; index++) {
        PoolModelIssue *issue = &model->issues[index];
        if (issue->occupied && !issue->published) {
            AmaPoolCell *cell = NULL;
            AmaStatus status = ama_pool_lookup(pool, issue->handle, true, &cell);
            assert(status.code == AMA_OK);
            publish_pool_cell(pool, issue->handle, cell);
            issue->published = true;
            return;
        }
    }
}
/** Release either the oldest or newest occupied identity, including abandoned private reservations. */
static void model_release(AmaPool *pool, PoolModel *model, bool newest) {
    for (size_t step = 0; step < model->issue_count; step++) {
        size_t index = newest ? model->issue_count - step - 1 : step;
        PoolModelIssue *issue = &model->issues[index];
        if (issue->occupied) {
            if (issue->published) { release_pool_cell(pool, issue->handle); }
            else {
                AmaStatus status = ama_pool_release(pool, issue->handle);
                assert(status.code == AMA_OK);
            }
            issue->occupied = false;
            return;
        }
    }
}
/** Reclaim exactly the oracle's empty materialized blocks without allocating or renumbering. */
static void model_reclaim(AmaPool *pool, PoolModel *model) {
    size_t expected = 0;
    for (size_t block = 0; block < model->block_count; block++) {
        if (model->present[block] && model_occupied(model, block, pool->slots_per_block) == 0) {
            expected++;
            model->present[block] = false;
        }
    }
    size_t allocations = allocation_count, released = SIZE_MAX;
    AmaStatus status = ama_pool_reclaim(pool, &released);
    assert(status.code == AMA_OK && released == expected && allocation_count == allocations);
}
