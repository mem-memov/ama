/** @file
 * @brief Select live candidates with a scalar boundary and no snapshots.
 * [FLOW-05](../../../../../spec/language/lists.md#flow-05) advances before
 * eligibility checks, skips only absent reference targets, and lets live busy
 * targets reach ordinary invocation refusal rather than silently filtering them.
 */
#include "internal.h"
#include "runtime/lifecycle/errors.h"

/** Borrow only target identity; activity does not decide scan eligibility. Component contract: [FLOW-05](../../../../../spec/language/lists.md#flow-05). */
static AmaStatus ama_foreach_target(AmaProgram *program, uint64_t type, AmaHandle target) {
    if (target.generation == 0) { return ama_status_make(AMA_EMPTY_REFERENCE, NULL, 0); }
    if (target.generation != 0 && target.type_id != type) { return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0); }
    AmaPoolCell *cell = NULL;
    return ama_program_cell(program, target, &cell);
}

AmaStatus ama_program_foreach_next(AmaProgram *program, AmaHandle owner,
    const AmaProgramForeachPlan *plan, bool *has_value) {
    *has_value = false;
    AmaForeachSlots slots;
    AmaProgramListService service = {0};
    AmaStatus status = ama_foreach_acquire(program, owner, plan, &slots, &service);
    if (status.code != AMA_OK) { return status; }
    AmaProgramForeachCursor *cursor = slots.cursor;
    AmaObjectType *owner_type = ama_world_type(program->world, owner.type_id);
    bool reference = owner_type->fields[plan->field].kind == AMA_FIELD_REFERENCE_LIST;
    if (cursor->boundary > service.list->count) { status = ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    while (status.code == AMA_OK && cursor->boundary < service.list->count) {
        AmaMembership membership;
        AmaHandle target;
        status = ama_membership_at(service.list, cursor->boundary, &membership, &target);
        if (status.code != AMA_OK) { break; }
        cursor->boundary++;
        status = ama_foreach_target(program, plan->element_type, target);
        if (reference && (status.code == AMA_EMPTY_REFERENCE || status.code == AMA_STALE_REFERENCE)) {
            status = ama_status_success();
            continue;
        }
        if (status.code != AMA_OK) { break; }
        if (!cursor->role_active && slots.binding->roles == SIZE_MAX) {
            status = ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0);
            break;
        }
        if (!cursor->role_active) { slots.binding->roles++; cursor->role_active = true; }
        *slots.value = target;
        slots.binding->origin = cursor->current = membership;
        slots.binding->installed = cursor->target = target;
        slots.binding->associated = true;
        *has_value = true;
        break;
    }
    status = ama_cleanup_primary(status, ama_program_list_close(&service));
    if (status.code != AMA_OK || *has_value) { return status; }
    return ama_program_foreach_leave(program, owner, plan, false);
}
