/**
 * @file
 * @brief Protect selected owned source trees without acquiring reference referents.
 *
 * [CALL-08](../../../../../spec/language/calls-and-results.md#call-08)
 * keeps supplied source boundaries stable while declaration-order installation proceeds.
 */
#include "internal.h"

AmaStatus ama_call_protect_sources(AmaObjectOperation *protection, const AmaPreparedInput *inputs, size_t input_count) {
    AmaStatus status = ama_status_success();
    /* Select all sources before acquiring any of them: repeated index readers
     * must observe the same pre-reset state and cannot encounter our own lease. */
    for (size_t index = 0; index < input_count; index++) {
        const AmaProgramValue *value = &inputs[index].value;
        if (inputs[index].input->source.kind == AMA_SOURCE_LITERAL || value->constant != NULL || value->shared_list != NULL) { continue; }
        if (value->form == AMA_FIELD_OWNED) { status = ama_object_preflight(protection, value->handle, true, false); }
        else if (value->form == AMA_FIELD_OWNING_LIST) {
            for (size_t element = 0; element < value->list->count && status.code == AMA_OK; element++) {
                status = ama_object_preflight(protection, value->list->items[element], true, false);
            }
        }
        if (status.code != AMA_OK) { return status; }
    }
    return status;
}
