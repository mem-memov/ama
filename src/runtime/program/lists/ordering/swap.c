/** @file
 * @brief Exchange membership positions after both exact integer conversions.
 * [LIST-08](../../../../../spec/language/lists.md#list-08) changes order without
 * acquiring targets or consuming their lifecycle credits, including self-swap.
 * [LIST-10](../../../../../spec/language/lists.md#list-10) keeps preflight refusal
 * distinct from native technical errors.
 */
#include "ordering.h"

AmaStatus ama_program_list_swap(AmaProgramListService *service, AmaProgramListMethod *method,
    const AmaProgramListInputs *plan, bool *refused) {
    *refused = false;
    AmaStatus status = ama_program_list_method_begin(service, method);
    if (status.code != AMA_OK) { return status; }
    static const AmaFieldKind inputs[] = {AMA_FIELD_OWNED, AMA_FIELD_OWNED};
    status = ama_program_list_prepare_inputs(method, plan, inputs, 2);
    if (status.code != AMA_OK) { return status; }
    AmaPoolCell *caller = NULL;
    status = ama_program_cell(service->program, service->caller, &caller);
    if (status.code != AMA_OK) { return status; }
    caller->metadata.service.current_operation = plan->operation;
    size_t first = SIZE_MAX, second = SIZE_MAX;
    status = ama_program_list_index(method, plan->integer_type, 0, &first, refused);
    if (status.code == AMA_OK) {
        status = ama_program_list_index(method, plan->integer_type, 1, &second, refused);
    }
    if (status.code == AMA_OK) {
        service->program->world->operation_active = true;
        status = ama_membership_swap(service->list, first, second);
        service->program->world->operation_active = false;
        *refused = status.code == AMA_INDEX_BOUNDS;
    }
    return ama_program_list_record_refusal(method, status, refused);
}
