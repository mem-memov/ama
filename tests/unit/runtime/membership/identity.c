/** @file
 * @brief Test duplicate occurrences, shifts, stable records and exact namespaces.
 */
#include "common.c"

/** Distinguish equal targets and reject retired generations after slot reuse. */
static void it_distinguishes_duplicate_targets_and_rejects_reused_occurrences(void) {
    setup_test();
    AmaMembershipList list = new_list(UINT64_MAX);
    AmaHandle target = {7, 4, 3}, other = {7, 5, 1};
    AmaMembership first, second, fresh;
    AmaStatus status = ama_membership_insert(&list, 0, target, &first);
    assert(status.code == AMA_OK);
    status = ama_membership_insert(&list, 1, target, &second);
    assert(status.code == AMA_OK && first.record != second.record);
    status = ama_membership_remove(&list, 0, 1);
    assert(status.code == AMA_OK);
    status = ama_membership_insert(&list, 0, other, &fresh);
    assert(status.code == AMA_OK && fresh.record == first.record && fresh.generation != first.generation);
    expect_stale(&list, first);
    const AmaMembershipRecord *record = NULL;
    status = ama_membership_lookup(&list, second, &record);
    assert(status.code == AMA_OK && record->position == 1);
    status = ama_membership_swap(&list, 0, 1);
    assert(status.code == AMA_OK);
    AmaHandle expected[] = {target, other};
    expect_order(&list, expected, 2);
    status = ama_membership_lookup(&list, second, &record);
    assert(status.code == AMA_OK && record->position == 0);
    AmaMembership wrong = second;
    wrong.owner.type_id++; expect_stale(&list, wrong);
    wrong = second; wrong.owner.slot_id++; expect_stale(&list, wrong);
    wrong = second; wrong.owner.generation++; expect_stale(&list, wrong);
    wrong = second; wrong.field++; expect_stale(&list, wrong);
    wrong = second; wrong.incarnation++; expect_stale(&list, wrong);
    wrong = second; wrong.generation = 0; expect_stale(&list, wrong);
    wrong = second; wrong.record = SIZE_MAX; expect_stale(&list, wrong);
    ama_membership_dispose(&list);
    destroy_test();
}

/** Force both directory and order growth while a borrowed record stays stable. */
static void it_preserves_record_addresses_and_bijections_through_growth_and_edits(void) {
    setup_test();
    AmaMembershipList list = new_list(UINT64_MAX);
    AmaMembership identity, scratch;
    AmaHandle targets[161];
    targets[0] = (AmaHandle){7, 0, 1};
    AmaStatus status = ama_membership_insert(&list, 0, targets[0], &identity);
    assert(status.code == AMA_OK);
    const AmaMembershipRecord *original = NULL;
    status = ama_membership_lookup(&list, identity, &original);
    assert(status.code == AMA_OK);
    for (size_t index = 1; index < 160; index++) {
        targets[index] = (AmaHandle){7, index, 1};
        status = ama_membership_insert(&list, index, targets[index], &scratch);
        assert(status.code == AMA_OK);
    }
    const AmaMembershipRecord *current = NULL;
    status = ama_membership_lookup(&list, identity, &current);
    assert(status.code == AMA_OK && current == original);
    expect_order(&list, targets, 160);
    AmaHandle inserted = {7, 999, 1};
    memmove(targets + 18, targets + 17, 143 * sizeof(*targets));
    targets[17] = inserted;
    status = ama_membership_insert(&list, 17, inserted, &scratch);
    assert(status.code == AMA_OK);
    expect_order(&list, targets, 161);
    status = ama_membership_remove(&list, 15, 50);
    assert(status.code == AMA_OK);
    memmove(targets + 15, targets + 65, 96 * sizeof(*targets));
    expect_order(&list, targets, 111);
    status = ama_membership_swap(&list, 1, 110);
    assert(status.code == AMA_OK);
    AmaHandle temporary = targets[1]; targets[1] = targets[110]; targets[110] = temporary;
    expect_order(&list, targets, 111);
    ama_membership_dispose(&list);
    destroy_test();
}

/** Replacement preserves same handles and makes changed/empty handles fresh. */
static void it_replaces_reference_memberships_atomically_and_resets_incarnations(void) {
    setup_test();
    AmaMembershipList list = new_list(3);
    AmaHandle target = {7, 4, 3};
    AmaMembership old, same, changed, after_reset;
    AmaStatus status = ama_membership_insert(&list, 0, target, &old);
    assert(status.code == AMA_OK);
    size_t allocations = allocation_count;
    status = ama_membership_replace(&list, 0, target, &same);
    assert(status.code == AMA_OK && same.record == old.record && same.generation == old.generation);
    assert(allocation_count == allocations && list.count == 1);
    status = ama_membership_replace(&list, 0, (AmaHandle){0}, &changed);
    assert(status.code == AMA_OK && list.count == 1);
    expect_stale(&list, old);
    AmaHandle empty[] = {{0}};
    expect_order(&list, empty, 1);
    status = ama_membership_reset(&list);
    assert(status.code == AMA_OK && list.count == 0 && list.incarnation == 2);
    expect_stale(&list, changed);
    status = ama_membership_insert(&list, 0, target, &after_reset);
    assert(status.code == AMA_OK);
    status = ama_membership_reset(&list);
    assert(status.code == AMA_OK && list.incarnation == 3);
    expect_stale(&list, after_reset);
    status = ama_membership_insert(&list, 0, target, &after_reset);
    assert(status.code == AMA_OK);
    status = ama_membership_reset(&list);
    assert(status.code == AMA_LIMIT_EXCEEDED && list.incarnation == 3);
    expect_order(&list, &target, 1);
    ama_membership_dispose(&list);
    destroy_test();
}
