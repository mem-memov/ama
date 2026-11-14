/** @file
 * @brief Construct the ephemeral integer length directly in fresh stable storage.
 * [LIST-04](../../../../../spec/language/lists.md#list-04) requires one constructor
 * after ordinary Ama fields; [PROTO-03](../../../../../spec/implementation/native-interface.md#proto-03)
 * supplies the exact typed host-size bridge, never literal parsing or overwrite.
 */
#include "ordering.h"

AmaStatus ama_program_list_length(AmaProgramListService *service, AmaProgramListMethod *method,
    const AmaProgramListInputs *plan, bool *refused) {
    *refused = false;
    AmaStatus status = ama_program_list_method_begin(service, method);
    if (status.code != AMA_OK) { return status; }
    status = ama_program_list_prepare_inputs(method, plan, NULL, 0);
    if (status.code != AMA_OK) { return status; }
    if (plan->from_size == NULL) { return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0); }
    AmaPoolCell *caller = NULL;
    status = ama_program_cell(service->program, service->caller, &caller);
    if (status.code != AMA_OK) { return status; }
    caller->metadata.service.current_operation = plan->operation;
    const AmaRecipe recipe = {.constructor = AMA_CONSTRUCT_SIZE,
        .size_value = service->list->count, .from_size = plan->from_size};
    return ama_program_list_owned_result(method, plan->integer_type, &recipe);
}
