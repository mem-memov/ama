/** @file
 * @brief Allocate stable occurrence blocks and movable compact order capacity.
 * [LIST-03](../../../../../spec/language/lists.md#list-03) permits size_t local
 * indices; [LIST-10](../../../../../spec/language/lists.md#list-10) requires
 * allocation failure before any publication of structural effects.
 */
#include <string.h>
#include "../internal.h"

/** Grow an explicitly typed byte allocation geometrically with checked products.
 * A failed allocation preserves its old buffer; successful preparation may retain
 * spare capacity even if another preparation step subsequently fails.
 */
static AmaStatus ama_membership_buffer(AmaMembershipList *list, void *old,
    size_t used, size_t capacity, size_t required, size_t width, void **result, size_t *new_capacity) {
    size_t limit = SIZE_MAX / width;
    if (required > limit) { return ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0); }
    if (required <= capacity) { *result = old; *new_capacity = capacity; return ama_status_success(); }
    size_t next = capacity == 0 ? (limit < 4 ? limit : 4) : capacity;
    while (next < required) { next = next > limit / 2 ? limit : next * 2; }
    void *buffer = list->allocator.allocate(list->allocator.context, next * width);
    if (buffer == NULL) { return ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0); }
    if (used != 0) { memcpy(buffer, old, used * width); }
    list->allocator.release(list->allocator.context, old);
    *result = buffer;
    *new_capacity = next;
    return ama_status_success();
}

AmaStatus ama_membership_prepare(AmaMembershipList *list, size_t required) {
    void *buffer = NULL;
    size_t capacity = 0;
    AmaStatus status = ama_membership_buffer(list, list->order, list->count,
        list->capacity, required, sizeof(*list->order), &buffer, &capacity);
    if (status.code != AMA_OK) { return status; }
    list->order = buffer;
    list->capacity = capacity;
    if (list->free_head != SIZE_MAX) { return ama_status_success(); }
    if (list->block_count >= SIZE_MAX / AMA_MEMBERSHIP_BLOCK_SIZE) {
        return ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0);
    }
    status = ama_membership_buffer(list, list->blocks, list->block_count,
        list->directory_capacity, list->block_count + 1, sizeof(*list->blocks), &buffer, &capacity);
    if (status.code != AMA_OK) { return status; }
    list->blocks = buffer;
    list->directory_capacity = capacity;
    AmaMembershipRecord *block = list->allocator.allocate(list->allocator.context,
        AMA_MEMBERSHIP_BLOCK_SIZE * sizeof(*block));
    if (block == NULL) { return ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0); }
    size_t base = list->block_count * AMA_MEMBERSHIP_BLOCK_SIZE;
    for (size_t index = 0; index < AMA_MEMBERSHIP_BLOCK_SIZE; index++) {
        block[index] = (AmaMembershipRecord){.generation = 1,
            .next_free = index + 1 == AMA_MEMBERSHIP_BLOCK_SIZE ? SIZE_MAX : base + index + 1};
    }
    list->blocks[list->block_count++] = block;
    list->free_head = base;
    return ama_status_success();
}

size_t ama_membership_take(AmaMembershipList *list, size_t position, AmaHandle target) {
    size_t index = list->free_head;
    AmaMembershipRecord *record = ama_membership_record(list, index);
    list->free_head = record->next_free;
    record->target = target;
    record->position = position;
    record->live = true;
    record->next_free = SIZE_MAX;
    return index;
}

void ama_membership_retire(AmaMembershipList *list, size_t index) {
    AmaMembershipRecord *record = ama_membership_record(list, index);
    record->live = false;
    record->target = (AmaHandle){0};
    record->position = SIZE_MAX;
    if (record->generation < list->counter_limit) {
        record->generation++;
        record->next_free = list->free_head;
        list->free_head = index;
    }
}
