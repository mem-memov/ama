/**
 * @file
 * @brief Release list handles and owned roots in reverse initialized-prefix order.
 *
 * [COPY-02](../../../../../spec/language/values-and-references.md#copy-02)
 * preserves the primary status while cleaning completed staging or old storage.
 */
#include "internal.h"

AmaStatus ama_transfer_list_discard(AmaProgram *program, AmaHandleList *list, bool owning, AmaStatus primary) {
    while (list->count != 0) {
        AmaHandle handle = list->items[--list->count];
        if (owning) { primary = ama_object_cleanup(program->world, handle, primary); }
    }
    program->world->allocator.release(program->world->allocator.context, list->items);
    *list = (AmaHandleList){0};
    return primary;
}
