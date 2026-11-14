/** @file
 * @brief Establish and discard one lexical scan without retaining protection.
 * [FLOW-05](../../../../../spec/language/lists.md#flow-05) leaves empty local
 * cells inactive and preserves reused references after exit; referenced targets
 * carry no cleanup credit and are never destroyed by these operations.
 */
#include "internal.h"

AmaStatus ama_program_foreach_empty(AmaProgram *program, AmaHandle owner, const AmaProgramForeachPlan *plan) {
    AmaProgramListService service = {0};
    bool refused;
    AmaStatus status = ama_program_list_open(program, owner, owner, plan->field, &service, &refused);
    if (status.code != AMA_OK) { return status; }
    AmaObjectType *type = ama_world_type(program->world, owner.type_id);
    bool matches = type->fields[plan->field].type_id == plan->element_type;
    status = ama_program_list_close(&service);
    return status.code == AMA_OK && !matches ? ama_status_make(AMA_TYPE_MISMATCH, NULL, 0) : status;
}

AmaStatus ama_program_foreach_enter(AmaProgram *program, AmaHandle owner, const AmaProgramForeachPlan *plan) {
    AmaForeachSlots slots;
    AmaStatus status = ama_foreach_slots(program, owner, plan, &slots);
    if (status.code != AMA_OK) { return status; }
    if (slots.cursor->active) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    AmaProgramListService service = {0};
    bool refused;
    status = ama_program_list_open(program, owner, owner, plan->field, &service, &refused);
    if (status.code != AMA_OK) { return status; }
    uint64_t incarnation = service.incarnation;
    status = ama_program_list_close(&service);
    if (status.code == AMA_OK) {
        *slots.cursor = (AmaProgramForeachCursor){.owner = owner, .field = plan->field,
            .incarnation = incarnation, .active = true};
    }
    return status;
}

AmaStatus ama_program_foreach_leave(AmaProgram *program, AmaHandle owner,
    const AmaProgramForeachPlan *plan, bool abandoning) {
    AmaForeachSlots slots;
    AmaStatus status = ama_foreach_slots(program, owner, plan, &slots);
    if (status.code != AMA_OK) { return status; }
    if (!slots.cursor->active) { return abandoning ? ama_status_success() : ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    if (!ama_handle_equal(slots.cursor->owner, owner) || slots.cursor->field != plan->field) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    if (slots.cursor->role_active) {
        if (slots.binding->roles == 0) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
        slots.binding->roles--;
        if (slots.binding->roles == 0) {
            *slots.binding = (AmaProgramForeachBinding){0};
        }
    }
    *slots.cursor = (AmaProgramForeachCursor){0};
    return ama_status_success();
}

AmaStatus ama_program_foreach_value(AmaProgram *program, AmaHandle owner,
    const AmaProgramForeachPlan *plan, AmaProgramValue *output) {
    AmaForeachSlots slots;
    AmaStatus status = ama_foreach_slots(program, owner, plan, &slots);
    if (status.code != AMA_OK) { return status; }
    if (plan->local && slots.binding->roles == 0) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    *output = (AmaProgramValue){.type_id = plan->element_type, .form = AMA_FIELD_REFERENCE,
        .container = owner, .location = slots.value, .handle = *slots.value,
        .foreach_binding = slots.binding};
    return ama_status_success();
}
