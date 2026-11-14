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
        AmaHandle handle = *ama_handle_list_element(list, list->count - 1);
        (void)ama_membership_remove(list, list->count - 1, 1);
        if (owning) { primary = ama_object_cleanup(program->world, handle, primary); }
    }
    ama_membership_dispose(list);
    return primary;
}
