/** @file
 * @brief Expose typed append/insert entry points without a runtime action selector.
 * [LIST-04](../../../../../spec/language/lists.md#list-04) gives each method its
 * own fixed signature; [LIST-09](../../../../../spec/language/lists.md#list-09)
 * chooses default construction, owning clone or complete reference copying.
 */
#include "internal.h"

/** Own one contiguous method prefix before returning to label-based outputs.
 * On any error, stable method credits remain available for chain/terminal cleanup.
 * Component contract: [LIST-04](../../../../../spec/language/lists.md#list-04), [LIST-09](../../../../../spec/language/lists.md#list-09). */
static AmaStatus ama_list_insertion(AmaProgramListService *service, AmaProgramListMethod *method,
    const AmaProgramListInputs *plan, bool positioned, bool *refused) {
    *refused = false;
    AmaStatus status = ama_program_list_method_begin(service, method);
    if (status.code != AMA_OK) { return status; }
    static const AmaFieldKind append_inputs[] = {AMA_FIELD_REFERENCE};
    static const AmaFieldKind insert_inputs[] = {AMA_FIELD_OWNED, AMA_FIELD_REFERENCE};
    status = ama_program_list_prepare_inputs(method, plan,
        positioned ? insert_inputs : append_inputs, positioned ? 2 : 1);
    return status.code == AMA_OK ? ama_list_insertion_commit(method, plan, positioned, refused) : status;
}

AmaStatus ama_program_list_append(AmaProgramListService *service, AmaProgramListMethod *method,
    const AmaProgramListInputs *plan, bool *refused) {
    return ama_list_insertion(service, method, plan, false, refused);
}

AmaStatus ama_program_list_insert(AmaProgramListService *service, AmaProgramListMethod *method,
    const AmaProgramListInputs *plan, bool *refused) {
    return ama_list_insertion(service, method, plan, true, refused);
}
