/** @file
 * @brief Validate full occurrence identities before exposing stable records.
 * [LIST-03](../../../../spec/language/lists.md#list-03) requires O(1) lookup
 * and distinguishes saved identities from protected current order indices.
 */
#include "internal.h"

AmaStatus ama_membership_lookup(const AmaMembershipList *list, AmaMembership identity,
    const AmaMembershipRecord **result) {
    if (!ama_handle_equal(identity.owner, list->owner) || identity.field != list->field ||
        identity.incarnation != list->incarnation || identity.generation == 0 ||
        identity.record / AMA_MEMBERSHIP_BLOCK_SIZE >= list->block_count) {
        return ama_status_make(AMA_STALE_REFERENCE, NULL, 0);
    }
    const AmaMembershipRecord *record = ama_membership_record(list, identity.record);
    if (!record->live || record->generation != identity.generation) {
        return ama_status_make(AMA_STALE_REFERENCE, NULL, 0);
    }
    *result = record;
    return ama_status_success();
}

AmaStatus ama_membership_at(const AmaMembershipList *list, size_t position,
    AmaMembership *identity, AmaHandle *target) {
    if (position >= list->count) { return ama_status_make(AMA_INDEX_BOUNDS, NULL, 0); }
    size_t index = list->order[position];
    *identity = ama_membership_identity(list, index);
    *target = ama_membership_record(list, index)->target;
    return ama_status_success();
}
