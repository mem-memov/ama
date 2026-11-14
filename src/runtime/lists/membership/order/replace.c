/** @file
 * @brief Replace one reference occurrence without a synthetic removal/insertion.
 * [LIST-13](../../../../../spec/language/lists.md#list-13) preserves length and
 * boundary positions, keeps same-handle identity and retires different handles.
 */
#include "../internal.h"

AmaStatus ama_membership_replace(AmaMembershipList *list, size_t position,
    AmaHandle target, AmaMembership *identity) {
    if (position >= list->count) { return ama_status_make(AMA_INDEX_BOUNDS, NULL, 0); }
    size_t old = list->order[position];
    if (ama_handle_equal(ama_membership_record(list, old)->target, target)) {
        *identity = ama_membership_identity(list, old);
        return ama_status_success();
    }
    AmaStatus status = ama_membership_prepare(list, list->count);
    if (status.code != AMA_OK) { return status; }
    size_t fresh = ama_membership_take(list, position, target);
    ama_membership_retire(list, old);
    list->order[position] = fresh;
    *identity = ama_membership_identity(list, fresh);
    return ama_status_success();
}
