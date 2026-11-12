/**
 * @file
 * @brief Select exactly one list element after checked index conversion.
 *
 * [CALL-08](../../../../../spec/language/calls-and-results.md#call-08)
 * distinguishes target bounds refusal from a technical index decoder failure.
 */
#include "internal.h"

AmaStatus ama_program_index(AmaProgramValue *value, size_t index) {
    bool reference = value->form == AMA_FIELD_REFERENCE_LIST;
    if (value->form != AMA_FIELD_OWNING_LIST && !reference) { return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0); }
    size_t count = value->shared_list != NULL ? value->shared_list->count : value->list->count;
    if (index >= count) { return ama_status_make(AMA_INDEX_BOUNDS, NULL, 0); }
    if (value->shared_list != NULL) {
        if (!reference) { value->constant = &value->shared_list->items[index]; }
        value->shared_list = NULL;
    } else { value->location = &value->list->items[index]; value->handle = *value->location; value->list = NULL; }
    value->form = reference ? AMA_FIELD_REFERENCE : AMA_FIELD_OWNED;
    return ama_status_success();
}
