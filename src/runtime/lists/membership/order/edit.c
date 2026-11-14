/** @file
 * @brief Publish exact insert/remove/swap order and inverse record positions.
 * [LIST-03](../../../../../spec/language/lists.md#list-03) requires a bijection;
 * [LIST-13](../../../../../spec/language/lists.md#list-13) preserves surviving
 * identities through shifts and swaps. Caller authority excludes observers.
 */
#include <string.h>
#include "../internal.h"

AmaStatus ama_membership_insert(AmaMembershipList *list, size_t position,
    AmaHandle target, AmaMembership *identity) {
    if (position > list->count) { return ama_status_make(AMA_INDEX_BOUNDS, NULL, 0); }
    if (list->count == SIZE_MAX) { return ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0); }
    AmaStatus status = ama_membership_prepare(list, list->count + 1);
    if (status.code != AMA_OK) { return status; }
    size_t index = ama_membership_take(list, position, target);
    memmove(list->order + position + 1, list->order + position,
        (list->count - position) * sizeof(*list->order));
    list->order[position] = index;
    list->count++;
    for (size_t offset = position + 1; offset < list->count; offset++) {
        ama_membership_record(list, list->order[offset])->position = offset;
    }
    *identity = ama_membership_identity(list, index);
    return ama_status_success();
}

AmaStatus ama_membership_remove(AmaMembershipList *list, size_t position, size_t count) {
    if (position > list->count || count > list->count - position) {
        return ama_status_make(AMA_INDEX_BOUNDS, NULL, 0);
    }
    if (count == 0) { return ama_status_success(); }
    for (size_t index = 0; index < count; index++) {
        ama_membership_retire(list, list->order[position + index]);
    }
    memmove(list->order + position, list->order + position + count,
        (list->count - position - count) * sizeof(*list->order));
    list->count -= count;
    for (size_t offset = position; offset < list->count; offset++) {
        ama_membership_record(list, list->order[offset])->position = offset;
    }
    return ama_status_success();
}

AmaStatus ama_membership_swap(AmaMembershipList *list, size_t first, size_t second) {
    if (first >= list->count || second >= list->count) {
        return ama_status_make(AMA_INDEX_BOUNDS, NULL, 0);
    }
    size_t temporary = list->order[first];
    list->order[first] = list->order[second];
    list->order[second] = temporary;
    ama_membership_record(list, list->order[first])->position = first;
    ama_membership_record(list, list->order[second])->position = second;
    return ama_status_success();
}
