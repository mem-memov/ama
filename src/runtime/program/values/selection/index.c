/**
 * @file
 * @brief Select exactly one list element after checked index conversion.
 * [LIST-13](../../../../../spec/language/lists.md#list-13) requires exact occurrence retirement and replacement namespaces.
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
    } else {
        AmaMembership identity;
        AmaHandle target;
        AmaStatus status = ama_membership_at(value->list, index, &identity, &target);
        if (status.code != AMA_OK) { return status; }
        const AmaMembershipRecord *record = NULL;
        status = ama_membership_lookup(value->list, identity, &record);
        if (status.code != AMA_OK) { return status; }
        value->membership_list = value->list;
        value->membership = identity;
        value->location = ama_handle_list_element(value->list, record->position);
        value->handle = target;
        value->list = NULL;
    }
    value->form = reference ? AMA_FIELD_REFERENCE : AMA_FIELD_OWNED;
    return ama_status_success();
}
