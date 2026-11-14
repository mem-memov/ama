/** @file
 * @brief Prepare declared list shapes using exact membership storage.
 * [LIST-03](../../../spec/language/lists.md#list-03) requires indirect records;
 * [LIST-13](../../../spec/language/lists.md#list-13) preserves namespace history
 * through replacement. Private partial shapes own only metadata until populated.
 */
#include "runtime/objects/objects.h"
#include "membership/internal.h"

AmaHandle *ama_handle_list_element(const AmaHandleList *list, size_t position) {
    return &ama_membership_record(list, list->order[position])->target;
}

AmaStatus ama_handle_list_prepare(AmaHandleList *list, SourceAllocator allocator,
    AmaHandle owner, size_t field, uint64_t incarnation, uint64_t limit, size_t count) {
    if (count > SIZE_MAX / sizeof(size_t) || incarnation == 0 || incarnation > limit) {
        return ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0);
    }
    AmaStatus status = ama_membership_init(list, allocator, owner, field, limit);
    if (status.code != AMA_OK) { return status; }
    list->incarnation = incarnation;
    for (size_t index = 0; index < count; index++) {
        AmaMembership identity;
        status = ama_membership_insert(list, index, (AmaHandle){0}, &identity);
        if (status.code != AMA_OK) { return status; }
    }
    return ama_status_success();
}
