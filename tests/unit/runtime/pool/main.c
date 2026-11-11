#include "../../source/common.c"
#include "runtime/pool/pool.h"

static void it_keeps_payload_addresses_stable_through_directory_growth(void) {
    setup_test();
    AmaPool pool;
    AmaStatus status = ama_pool_init(&pool, sources.allocator, 7, sizeof(size_t), 2, UINT64_MAX);
    assert(status.code == AMA_OK && pool.live_count == 0);
    AmaHandle handles[50];
    size_t *addresses[50];
    for (size_t index = 0; index < 50; index++) {
        AmaPoolCell *cell = NULL;
        status = ama_pool_reserve(&pool, &handles[index], &cell);
        assert(status.code == AMA_OK);
        addresses[index] = ama_pool_payload(&pool, cell);
        *addresses[index] = index;
    }
    for (size_t index = 0; index < 50; index++) {
        AmaPoolCell *cell = NULL;
        status = ama_pool_lookup(&pool, handles[index], true, &cell);
        assert(status.code == AMA_OK && addresses[index] == ama_pool_payload(&pool, cell));
        assert(*addresses[index] == index);
        status = ama_pool_release(&pool, handles[index]);
        assert(status.code == AMA_OK);
    }
    ama_pool_dispose(&pool);
    destroy_test();
}

static void it_never_reissues_an_exhausted_slot_or_accepts_stale_empty_and_wrong_type_handles(void) {
    setup_test();
    AmaPool pool;
    AmaStatus status = ama_pool_init(&pool, sources.allocator, 7, 1, 1, 3);
    assert(status.code == AMA_OK);
    AmaHandle history[9];
    for (size_t index = 0; index < 9; index++) {
        AmaPoolCell *cell = NULL;
        status = ama_pool_reserve(&pool, &history[index], &cell);
        assert(status.code == AMA_OK && history[index].slot_id == index / 3);
        cell->metadata.lifetime.phase = AMA_LIFETIME_LIVE;
        status = ama_pool_publish(&pool, history[index]);
        assert(status.code == AMA_OK && pool.live_count == 1);
        for (size_t old = 0; old < index; old++) {
            status = ama_pool_lookup(&pool, history[old], false, &cell);
            assert(status.code == AMA_STALE_REFERENCE);
        }
        status = ama_pool_lookup(&pool, (AmaHandle){0}, false, &cell);
        assert(status.code == AMA_EMPTY_REFERENCE);
        AmaHandle wrong = history[index];
        wrong.type_id++;
        status = ama_pool_lookup(&pool, wrong, false, &cell);
        assert(status.code == AMA_TYPE_MISMATCH);
        status = ama_pool_invalidate(&pool, history[index]);
        assert(status.code == AMA_OK);
        status = ama_pool_lookup(&pool, history[index], true, &cell);
        assert(status.code == AMA_OK);
        cell->metadata.lifetime.phase = AMA_LIFETIME_FINALIZED;
        status = ama_pool_release(&pool, history[index]);
        assert(status.code == AMA_OK && pool.live_count == 0);
    }
    ama_pool_dispose(&pool);
    destroy_test();
}

static void it_preserves_pool_validity_when_each_growth_allocation_fails(void) {
    for (size_t failure = 1; failure <= 2; failure++) {
        setup_test();
        AmaPool pool;
        AmaStatus status = ama_pool_init(&pool, sources.allocator, 1, 8, 2, 2);
        assert(status.code == AMA_OK);
        failure_at = failure;
        AmaHandle handle = {0};
        AmaPoolCell *cell = NULL;
        status = ama_pool_reserve(&pool, &handle, &cell);
        assert(status.code == AMA_OUT_OF_MEMORY && pool.live_count == 0 && pool.block_count == 0);
        failure_at = 0;
        status = ama_pool_reserve(&pool, &handle, &cell);
        assert(status.code == AMA_OK);
        ama_pool_dispose(&pool);
        destroy_test();
    }
}

void run(void) {
    it_keeps_payload_addresses_stable_through_directory_growth();
    it_never_reissues_an_exhausted_slot_or_accepts_stale_empty_and_wrong_type_handles();
    it_preserves_pool_validity_when_each_growth_allocation_fails();
}

int main(void) {
    run();
    return EXIT_SUCCESS;
}
