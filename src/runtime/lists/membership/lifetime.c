/** @file
 * @brief Own collection metadata lifetime without owning referenced Ama values.
 * [LIST-13](../../../../spec/language/lists.md#list-13) makes reset a new
 * incarnation even for an empty collection and forbids counter wrap.
 */
#include "internal.h"

AmaStatus ama_membership_init(AmaMembershipList *list, SourceAllocator allocator,
    AmaHandle owner, size_t field, uint64_t limit) {
    if (owner.generation == 0 || owner.type_id == 0 || limit == 0 ||
        allocator.allocate == NULL || allocator.release == NULL) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    *list = (AmaMembershipList){.allocator = allocator, .owner = owner, .field = field,
        .incarnation = 1, .counter_limit = limit, .free_head = SIZE_MAX};
    return ama_status_success();
}

void ama_membership_dispose(AmaMembershipList *list) {
    if (list->allocator.release == NULL) { *list = (AmaMembershipList){0}; return; }
    for (size_t index = 0; index < list->block_count; index++) {
        list->allocator.release(list->allocator.context, list->blocks[index]);
    }
    list->allocator.release(list->allocator.context, list->blocks);
    list->allocator.release(list->allocator.context, list->order);
    *list = (AmaMembershipList){0};
}

AmaStatus ama_membership_reset(AmaMembershipList *list) {
    if (list->incarnation == list->counter_limit) {
        return ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0);
    }
    /* All required counter validation precedes this nonfailing publication. */
    for (size_t index = 0; index < list->count; index++) {
        ama_membership_retire(list, list->order[index]);
    }
    list->count = 0;
    list->incarnation++;
    return ama_status_success();
}
