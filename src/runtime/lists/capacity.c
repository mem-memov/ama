/**
 * @file
 * @brief Grow contiguous handle storage without relocating instance payloads.
 *
 * [RT-02](../../../spec/implementation/runtime.md#rt-02)
 * Checked geometric allocation preserves the old array on failure. This foundation does not supply membership identities or traversal cursors.
 */
#include <stdint.h>
#include <string.h>
#include "runtime/objects/objects.h"

/** Require a consistent handle array and allocator. Checked geometric growth allocates/copies before releasing old storage; failure preserves count, capacity and items. */
AmaStatus ama_handle_list_reserve(SourceAllocator allocator, AmaHandleList *list, size_t required) {
    size_t limit = SIZE_MAX / sizeof(AmaHandle);
    if (required > limit || list->count > list->capacity || list->capacity > limit) {
        return ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0);
    }
    if (required <= list->capacity) { return ama_status_success(); }
    size_t capacity = list->capacity == 0 ? 4 : list->capacity;
    if (capacity > limit) { capacity = limit; }
    while (capacity < required) { capacity = capacity > limit / 2 ? limit : capacity * 2; }
    AmaHandle *items = allocator.allocate(allocator.context, capacity * sizeof(*items));
    if (items == NULL) { return ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0); }
    if (list->count != 0) { memcpy(items, list->items, list->count * sizeof(*items)); }
    allocator.release(allocator.context, list->items);
    list->items = items;
    list->capacity = capacity;
    return ama_status_success();
}
