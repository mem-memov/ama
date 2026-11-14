/** @file
 * @brief Test prepared allocation failure, reduced counters and checked bounds.
 */
#include "common.c"

/** Exhaust record generations repeatedly without ever reviving a saved identity. */
static void it_permanently_retires_exhausted_record_generations(void) {
    setup_test();
    AmaMembershipList list = new_list(1);
    AmaMembership retired[70], current;
    AmaHandle target = {7, 4, 3};
    for (size_t index = 0; index < 70; index++) {
        AmaStatus status = ama_membership_insert(&list, 0, target, &current);
        assert(status.code == AMA_OK && current.generation == 1);
        retired[index] = current;
        for (size_t previous = 0; previous < index; previous++) {
            assert(retired[previous].record != current.record);
            expect_stale(&list, retired[previous]);
        }
        status = ama_membership_remove(&list, 0, 1);
        assert(status.code == AMA_OK);
        expect_stale(&list, current);
    }
    ama_membership_dispose(&list);
    destroy_test();
}

/** Fail each allocation needed by initial insertion, then recover without leaks. */
static void it_preserves_empty_state_and_output_on_every_initial_allocation_failure(void) {
    for (size_t failing = 1; failing <= 3; failing++) {
        setup_test();
        AmaMembershipList list = new_list(UINT64_MAX);
        AmaMembership output = {.generation = 99};
        failure_at = allocation_count + failing;
        AmaStatus status = ama_membership_insert(&list, 0, (AmaHandle){0}, &output);
        assert(status.code == AMA_OUT_OF_MEMORY && list.count == 0 && output.generation == 99);
        failure_at = 0;
        status = ama_membership_insert(&list, 0, (AmaHandle){0}, &output);
        assert(status.code == AMA_OK && output.generation == 1);
        ama_membership_dispose(&list);
        destroy_test();
    }
}

/** Fill a block to force replacement allocation; old saved identities and values
 * remain live after failure, and invalid ranges cannot overflow into publication.
 */
static void it_preserves_live_occurrences_on_failed_replacement_and_invalid_bounds(void) {
    setup_test();
    AmaMembershipList list = new_list(UINT64_MAX);
    AmaMembership identities[32], output = {.generation = 99};
    AmaHandle targets[32];
    for (size_t index = 0; index < 32; index++) {
        targets[index] = (AmaHandle){7, index, 1};
        AmaStatus status = ama_membership_insert(&list, index, targets[index], &identities[index]);
        assert(status.code == AMA_OK);
    }
    failure_at = allocation_count + 1;
    AmaStatus status = ama_membership_replace(&list, 2, (AmaHandle){0}, &output);
    assert(status.code == AMA_OUT_OF_MEMORY && output.generation == 99);
    expect_order(&list, targets, 32);
    for (size_t index = 0; index < 32; index++) {
        const AmaMembershipRecord *record = NULL;
        status = ama_membership_lookup(&list, identities[index], &record);
        assert(status.code == AMA_OK && record->position == index);
    }
    failure_at = 0;
    status = ama_membership_insert(&list, SIZE_MAX, (AmaHandle){0}, &output);
    assert(status.code == AMA_INDEX_BOUNDS && output.generation == 99);
    status = ama_membership_remove(&list, 1, SIZE_MAX);
    assert(status.code == AMA_INDEX_BOUNDS);
    status = ama_membership_swap(&list, 0, 32);
    assert(status.code == AMA_INDEX_BOUNDS);
    status = ama_membership_replace(&list, 32, (AmaHandle){0}, &output);
    assert(status.code == AMA_INDEX_BOUNDS);
    AmaHandle target = {0};
    status = ama_membership_at(&list, 32, &output, &target);
    assert(status.code == AMA_INDEX_BOUNDS && output.generation == 99);
    status = ama_membership_remove(&list, 32, 0);
    assert(status.code == AMA_OK);
    expect_order(&list, targets, 32);
    ama_membership_dispose(&list);
    destroy_test();
}
