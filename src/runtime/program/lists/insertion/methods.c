/** @file
 * @brief Expose typed append/insert entry points without a runtime action selector.
 * [LIST-04](../../../../../spec/language/lists.md#list-04) gives each method its
 * own fixed signature; [LIST-09](../../../../../spec/language/lists.md#list-09)
 * chooses default construction, owning clone or complete reference copying.
 */
#include "internal.h"

/** Own one contiguous method prefix before returning to label-based outputs.
 * On any error, stable method credits remain available for chain/terminal cleanup.
 */
static AmaStatus ama_list_insertion(AmaProgramListService *service, AmaProgramListMethod *method,
    const AmaProgramListInputs *plan, bool positioned, bool *refused) {
    *refused = false;
    AmaStatus status = ama_program_list_method_begin(service, method);
    if (status.code != AMA_OK) { return status; }
    AmaListInsertionInputs inputs = {.positioned = positioned, .arity = positioned ? 2 : 1,
        .source_position = positioned ? 1 : 0};
    status = ama_list_insertion_inputs(method, plan, &inputs);
    return status.code == AMA_OK ? ama_list_insertion_commit(method, plan, &inputs, refused) : status;
}

AmaStatus ama_program_list_append(AmaProgramListService *service, AmaProgramListMethod *method,
    const AmaProgramListInputs *plan, bool *refused) {
    return ama_list_insertion(service, method, plan, false, refused);
}

AmaStatus ama_program_list_insert(AmaProgramListService *service, AmaProgramListMethod *method,
    const AmaProgramListInputs *plan, bool *refused) {
    return ama_list_insertion(service, method, plan, true, refused);
}
