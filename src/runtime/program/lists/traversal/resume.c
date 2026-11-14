/** @file
 * @brief Validate saved scans outermost first and abandon only vanished iterations.
 * [FLOW-05](../../../../../spec/language/lists.md#flow-05) treats collection
 * replacement/acquisition failure as technical at its header, while a missing
 * current occurrence skips the old body from the corrected live boundary.
 */
#include "internal.h"

AmaStatus ama_program_foreach_reset(AmaProgram *program, AmaHandle owner) {
    const AmaProgramType *type = ama_program_type(program, owner.type_id);
    if (type == NULL) { return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0); }
    for (size_t index = type->foreach_count; index != 0;) {
        AmaStatus status = ama_program_foreach_leave(program, owner, &type->foreach_scopes[--index].plan, true);
        if (status.code != AMA_OK) { return status; }
    }
    return ama_status_success();
}

/** Parent indices strictly decrease, giving an exact finite ancestry test. */
static bool ama_foreach_descendant(const AmaProgramType *type, size_t child, size_t parent) {
    for (size_t index = type->foreach_scopes[child].parent; index != SIZE_MAX;
            index = type->foreach_scopes[index].parent) {
        if (index == parent) { return true; }
    }
    return false;
}

AmaStatus ama_program_foreach_resume(AmaProgram *program, AmaHandle owner, uint64_t *repeat) {
    *repeat = 0;
    if (program->world->operation_active || program->world->failed || !ama_handle_equal(program->current, owner)) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    const AmaProgramType *type = ama_program_type(program, owner.type_id);
    if (type == NULL) { return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0); }
    AmaPoolCell *cell = NULL;
    AmaStatus status = ama_program_cell(program, owner, &cell);
    if (status.code != AMA_OK) { return status; }
    for (size_t index = 0; index < type->foreach_count; index++) {
        const AmaProgramForeachScope *scope = &type->foreach_scopes[index];
        AmaForeachSlots slots;
        status = ama_foreach_slots(program, owner, &scope->plan, &slots);
        if (status.code != AMA_OK) { return status; }
        if (!slots.cursor->active) { continue; }
        cell->metadata.service.current_operation = scope->header;
        if (scope->parent != SIZE_MAX) {
            AmaForeachSlots parent;
            status = ama_foreach_slots(program, owner, &type->foreach_scopes[scope->parent].plan, &parent);
            if (status.code != AMA_OK) { return status; }
            if (!parent.cursor->active || !parent.cursor->role_active) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
        }
        AmaProgramListService service = {0};
        status = ama_foreach_acquire(program, owner, &scope->plan, &slots, &service);
        if (status.code != AMA_OK) { return status; }
        bool missing = false;
        if (slots.cursor->role_active) {
            const AmaMembership *current = &slots.cursor->current;
            if (!ama_handle_equal(current->owner, slots.cursor->owner) || current->field != slots.cursor->field
                || current->incarnation != slots.cursor->incarnation) { status = ama_status_make(AMA_INVALID_STATE, NULL, 0); }
            else { status = ama_program_foreach_origin(program, *current, slots.cursor->target, &service, &missing); }
        }
        AmaStatus closed = ama_program_list_close(&service);
        if (closed.code != AMA_OK) { return closed; }
        if (status.code != AMA_OK && !missing) { return status; }
        if (!missing) { continue; }
        for (size_t nested = type->foreach_count; nested > index + 1;) {
            size_t child = --nested;
            if (ama_foreach_descendant(type, child, index)) {
                status = ama_program_foreach_leave(program, owner, &type->foreach_scopes[child].plan, true);
                if (status.code != AMA_OK) { return status; }
            }
        }
        *repeat = scope->repeat;
        return ama_status_success();
    }
    return ama_status_success();
}
