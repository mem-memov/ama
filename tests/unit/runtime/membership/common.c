/** @file
 * @brief Own counted metadata allocation and independent order/identity assertions.
 */
#ifndef AMA_MEMBERSHIP_TEST_COMMON
#define AMA_MEMBERSHIP_TEST_COMMON
#include "../../source/common.c"
#include "runtime/lists/membership/membership.h"

/** Create one exact owner/field namespace with deliberately testable counters. */
static AmaMembershipList new_list(uint64_t limit) {
    AmaMembershipList list;
    AmaStatus status = ama_membership_init(&list, sources.allocator,
        (AmaHandle){4, 9, 2}, 3, limit);
    assert(status.code == AMA_OK);
    return list;
}

/** Independently check the published order-to-record bijection and target data. */
static void expect_order(const AmaMembershipList *list, const AmaHandle *targets, size_t count) {
    assert(list->count == count);
    for (size_t position = 0; position < count; position++) {
        AmaMembership identity;
        AmaHandle target;
        AmaStatus status = ama_membership_at(list, position, &identity, &target);
        assert(status.code == AMA_OK && ama_handle_equal(target, targets[position]));
        const AmaMembershipRecord *record = NULL;
        status = ama_membership_lookup(list, identity, &record);
        assert(status.code == AMA_OK && record->live && record->position == position);
        assert(ama_handle_equal(record->target, target));
        for (size_t previous = 0; previous < position; previous++) {
            assert(list->order[previous] != list->order[position]);
        }
    }
}

/** Verify failed identity validation publishes no borrowed record. */
static void expect_stale(const AmaMembershipList *list, AmaMembership identity) {
    AmaMembershipRecord marker = {0};
    const AmaMembershipRecord *record = &marker;
    AmaStatus status = ama_membership_lookup(list, identity, &record);
    assert(status.code == AMA_STALE_REFERENCE && record == &marker);
}
#endif
