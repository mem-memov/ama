/**
 * @file
 * @brief Unify prepared owned roots into one exact forest reference-map boundary.
 *
 * [COPY-05](../../../../../spec/language/values-and-references.md#copy-05)
 * remaps cross-element references only after all forest identities exist.
 */
#include "internal.h"

AmaStatus ama_transfer_list_fixup(AmaObjectOperation *operation, bool copied_source) {
    if (operation->first != NULL && copied_source) {
        for (AmaObjectWork *work = operation->first; work != NULL; work = work->next) { work->clone_root = operation->first; }
        return ama_object_fix_references(operation, operation->first, (AmaHandle){0}, false);
    }
    return ama_status_success();
}
