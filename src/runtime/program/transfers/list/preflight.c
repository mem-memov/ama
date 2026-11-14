/**
 * @file
 * @brief Check list transfer shape, aliases and source/destination availability.
 *
 * [COPY-06](../../../../../spec/language/values-and-references.md#copy-06)
 * checks nominal shape and ordinary self-copy before allocating staging storage.
 */
#include "internal.h"

/** Protect each owned root in order; caller restores any successful prefix. */
static AmaStatus ama_transfer_list_preflight(AmaObjectOperation *operation, const AmaHandleList *list, bool copy, bool continuation) {
    for (size_t index = 0; index < list->count; index++) {
        AmaStatus status = ama_object_preflight(operation, *ama_handle_list_element(list, index), copy, continuation);
        if (status.code != AMA_OK) { return status; }
    }
    return ama_status_success();
}

/** Validate before staging. An ordinary identical list is an allocation-free
 * no-op; continuation self-copy is invalid. Partial protections belong to caller.
 */
AmaStatus ama_transfer_list_validate(AmaProgramValue *destination, const AmaProgramValue *source,
    AmaObjectOperation *operation, bool *no_op) {
    *no_op = false;
    bool owning = destination->form == AMA_FIELD_OWNING_LIST;
    if (source != NULL && (destination->form != source->form || destination->type_id != source->type_id)) {
        return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0);
    }
    if (destination->list == NULL) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    if (source != NULL && destination->list == source->list) {
        *no_op = !source->continuation;
        return source->continuation ? ama_status_make(AMA_INVALID_STATE, NULL, 0) : ama_status_success();
    }
    AmaStatus status = ama_status_success();
    if (owning && source != NULL) {
        status = ama_object_copy_capability(operation, destination->type_id);
        if (status.code == AMA_OK && source->list != NULL) { status = ama_transfer_list_preflight(operation, source->list, true, source->continuation); }
    }
    if (status.code == AMA_OK && owning) { status = ama_transfer_list_preflight(operation, destination->list, false, false); }
    return status;
}
