/** @file
 * @brief Exhaust finite reserve/publish/release/reclaim schedules against an independent issued-identity ledger.
 */
/** One issued identity, whose expected state is independent of the pool's free chain and lifecycle metadata. */
typedef struct {
    AmaHandle handle;
    void *address;
    bool occupied;
    bool published;
} PoolModelIssue;
/** Six operations issue at most six identities/blocks; absent directory entries remain permanently absent in this implementation policy. */
typedef struct {
    PoolModelIssue issues[6];
    size_t issue_count;
    bool present[6];
    size_t block_count;
} PoolModel;
/** Latest issued generation at a physical slot, or no prior issuance. */
static const PoolModelIssue *model_previous(const PoolModel *model, size_t slot) {
    for (size_t index = model->issue_count; index != 0;) {
        const PoolModelIssue *issue = &model->issues[--index];
        if (issue->handle.slot_id == slot) { return issue; }
    }
    return NULL;
}
/** Count the oracle's occupied private/public identities within one historical block. */
static size_t model_occupied(const PoolModel *model, size_t block, size_t slots) {
    size_t count = 0;
    for (size_t index = 0; index < model->issue_count; index++) {
        const PoolModelIssue *issue = &model->issues[index];
        if (issue->occupied && issue->handle.slot_id / slots == block) { count++; }
    }
    return count;
}
/** The exact free set excludes occupied, exhausted and historically reclaimed identities. */
static bool model_free(const PoolModel *model, size_t slot, size_t slots, uint64_t limit) {
    size_t block = slot / slots;
    if (block >= model->block_count || !model->present[block]) { return false; }
    const PoolModelIssue *previous = model_previous(model, slot);
    return previous == NULL || (!previous->occupied && previous->handle.generation < limit);
}
/** Compare all issued handles, stable addresses, physical counts and the complete unique free chain after each transition. */
static void assert_pool_model(const AmaPool *pool, const PoolModel *model) {
    size_t live = 0, allocated = 0, expected_free = 0;
    assert(pool->block_count == model->block_count);
    for (size_t block = 0; block < model->block_count; block++) {
        assert((pool->blocks[block].memory != NULL) == model->present[block]);
        assert(pool->blocks[block].occupied == model_occupied(model, block, pool->slots_per_block));
        if (model->present[block]) { allocated++; }
        for (size_t offset = 0; offset < pool->slots_per_block; offset++) {
            if (model_free(model, block * pool->slots_per_block + offset, pool->slots_per_block, pool->generation_limit)) { expected_free++; }
        }
    }
    for (size_t index = 0; index < model->issue_count; index++) {
        const PoolModelIssue *issue = &model->issues[index];
        AmaPoolCell *cell = NULL;
        AmaStatus status = ama_pool_lookup(pool, issue->handle, true, &cell);
        assert(status.code == (issue->occupied ? AMA_OK : AMA_STALE_REFERENCE));
        if (issue->occupied) {
            assert(ama_pool_payload(pool, cell) == issue->address);
            assert(*(size_t *)issue->address == index + 10);
        }
        status = ama_pool_lookup(pool, issue->handle, false, &cell);
        assert(status.code == (issue->occupied && issue->published ? AMA_OK : AMA_STALE_REFERENCE));
        if (issue->occupied && issue->published) { live++; }
    }
    assert(pool->live_count == live && pool->allocated_blocks == allocated);
    bool seen[12] = {false};
    size_t free_count = 0;
    for (size_t slot = pool->free_head; slot != SIZE_MAX;) {
        assert(slot < 12 && !seen[slot]);
        assert(model_free(model, slot, pool->slots_per_block, pool->generation_limit));
        seen[slot] = true;
        free_count++;
        size_t block = slot / pool->slots_per_block;
        size_t offset = slot % pool->slots_per_block;
        const AmaPoolCell *cell = (const AmaPoolCell *)(const void *)(pool->blocks[block].memory + offset * pool->stride);
        slot = cell->next_free;
    }
    assert(free_count == expected_free);
}
