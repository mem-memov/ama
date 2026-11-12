/**
 * @file
 * @brief Check stable slots, checked growth, exact generations and permanent reclamation history.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../docs/implementation/library-tests-and-build.md)
 */
#include "../../source/common.c"
#include "runtime/pool/pool.h"

/**
 * Assert that the component keeps payload addresses stable through directory growth. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
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

/**
 * Assert that the component never reissues an exhausted slot or accepts stale empty and wrong
 * type handles. The explicit fixture setup and observations define this regression; assertion
 * failure keeps its generated evidence.
 */
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

/**
 * Assert that the component preserves pool validity when each growth allocation fails. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
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

/**
 * Assert that the component reclaims only free trailing blocks and never revives their
 * identities. The explicit fixture setup and observations define this regression; assertion
 * failure keeps its generated evidence.
 */
static void it_reclaims_only_free_trailing_blocks_and_never_revives_their_identities(void) {
    setup_test();
    AmaPool pool;
    AmaStatus status = ama_pool_init(&pool, sources.allocator, 9, sizeof(size_t), 2, 2);
    assert(status.code == AMA_OK);
    AmaHandle handles[4];
    AmaPoolCell *first = NULL;
    for (size_t index = 0; index < 4; index++) {
        AmaPoolCell *cell = NULL;
        status = ama_pool_reserve(&pool, &handles[index], &cell);
        assert(status.code == AMA_OK);
        if (index == 0) { first = cell; }
    }
    status = ama_pool_release(&pool, handles[1]);
    assert(status.code == AMA_OK);
    size_t released;
    status = ama_pool_reclaim(&pool, &released);
    assert(status.code == AMA_OK && released == 0);
    for (size_t index = 2; index < 4; index++) {
        status = ama_pool_release(&pool, handles[index]);
        assert(status.code == AMA_OK);
    }
    status = ama_pool_reclaim(&pool, &released);
    assert(status.code == AMA_OK && released == 1 && pool.allocated_blocks == 1);
    for (size_t index = 0; index < 20; index++) {
        AmaHandle next;
        AmaPoolCell *cell = NULL;
        status = ama_pool_reserve(&pool, &next, &cell);
        assert(status.code == AMA_OK);
        status = ama_pool_lookup(&pool, handles[0], true, &cell);
        assert(status.code == AMA_OK && cell == first);
        for (size_t old = 1; old < 4; old++) {
            status = ama_pool_lookup(&pool, handles[old], true, &cell);
            assert(status.code == AMA_STALE_REFERENCE);
        }
        status = ama_pool_release(&pool, next);
        assert(status.code == AMA_OK);
        status = ama_pool_reclaim(&pool, &released);
        assert(status.code == AMA_OK);
    }
    status = ama_pool_release(&pool, handles[0]);
    assert(status.code == AMA_OK);
    status = ama_pool_reclaim(&pool, &released);
    assert(status.code == AMA_OK && pool.allocated_blocks == 0);
    ama_pool_dispose(&pool);
    destroy_test();
}

/**
 * Run every explicitly selected assertion case in this executable; a failed assertion preserves
 * leaf generated output for diagnosis.
 */
void run(void) {
    it_reclaims_only_free_trailing_blocks_and_never_revives_their_identities();
    it_keeps_payload_addresses_stable_through_directory_growth();
    it_never_reissues_an_exhausted_slot_or_accepts_stale_empty_and_wrong_type_handles();
    it_preserves_pool_validity_when_each_growth_allocation_fails();
}

/**
 * Enter the explicit test run and return success only if all assertions complete; the parent
 * harness observes this process status.
 */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
