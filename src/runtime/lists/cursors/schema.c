/** @file
 * @brief Validate a fixed scan catalog before cursor pointer arithmetic.
 * [LIST-13](../../../../spec/language/lists.md#list-13) visits each lexical slot
 * once; strictly ascending actual offsets forbid duplicate correction of a slot.
 * [RT-02](../../../../spec/implementation/runtime.md#rt-02) requires aligned,
 * bounded compiler-visible storage rather than an opaque native pointer registry.
 */
#include "model.h"

AmaStatus ama_list_cursor_schema(size_t size, size_t fields, const AmaListCursorSet *sets) {
    if (sets == NULL) { return ama_status_success(); }
    for (size_t field = 0; field < fields; field++) {
        const AmaListCursorSet *set = &sets[field];
        if (set->count > size / sizeof(AmaListCursor) || (set->count != 0 && set->offsets == NULL)) {
            return ama_status_make(AMA_INVALID_STATE, NULL, 0);
        }
        for (size_t index = 0; index < set->count; index++) {
            size_t offset = set->offsets[index];
            if (offset % _Alignof(AmaListCursor) != 0 || offset > size || sizeof(AmaListCursor) > size - offset
                || (index != 0 && set->offsets[index - 1] + sizeof(AmaListCursor) > offset)) {
                return ama_status_make(AMA_INVALID_STATE, NULL, 0);
            }
        }
    }
    return ama_status_success();
}
