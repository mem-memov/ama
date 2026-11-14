/** @file
 * @brief Validate stable lexical scan offsets before exposing temporary views.
 * [RT-02](../../../../../spec/implementation/runtime.md#rt-02) uses generated
 * sizeof/offsetof rather than guessed layouts; [FLOW-05](../../../../../spec/language/lists.md#flow-05)
 * binds each cursor to its exact current-instance list field.
 */
#include "internal.h"

/** Subtraction-based extent check avoids overflowing offset plus size. */
static bool ama_foreach_extent(size_t extent, size_t offset, size_t size, size_t alignment) {
    return offset % alignment == 0 && offset <= extent && size <= extent - offset;
}

AmaStatus ama_foreach_slots(AmaProgram *program, AmaHandle owner,
    const AmaProgramForeachPlan *plan, AmaForeachSlots *output) {
    AmaPoolCell *cell = NULL;
    AmaStatus status = ama_program_cell(program, owner, &cell);
    if (status.code != AMA_OK) { return status; }
    AmaObjectType *type = ama_world_type(program->world, owner.type_id);
    if (plan->field >= type->field_count) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    const AmaObjectField *field = &type->fields[plan->field];
    if ((field->kind != AMA_FIELD_OWNING_LIST && field->kind != AMA_FIELD_REFERENCE_LIST)
        || field->type_id != plan->element_type) { return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0); }
    size_t extent = type->pool.payload_size;
    if (!ama_foreach_extent(extent, plan->cursor_offset, sizeof(AmaProgramForeachCursor), _Alignof(AmaProgramForeachCursor))
        || !ama_foreach_extent(extent, plan->binding_offset, sizeof(AmaProgramForeachBinding), _Alignof(AmaProgramForeachBinding))
        || !ama_foreach_extent(extent, plan->value_offset, sizeof(AmaHandle), _Alignof(AmaHandle))) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    unsigned char *base = ama_pool_payload(&type->pool, cell);
    *output = (AmaForeachSlots){.cursor = (void *)(base + plan->cursor_offset),
        .binding = (void *)(base + plan->binding_offset), .value = (void *)(base + plan->value_offset)};
    return ama_status_success();
}

AmaStatus ama_foreach_acquire(AmaProgram *program, AmaHandle owner, const AmaProgramForeachPlan *plan,
    AmaForeachSlots *slots, AmaProgramListService *service) {
    AmaStatus status = ama_foreach_slots(program, owner, plan, slots);
    if (status.code != AMA_OK) { return status; }
    const AmaProgramForeachCursor *cursor = slots->cursor;
    if (!cursor->active || !ama_handle_equal(cursor->owner, owner) || cursor->field != plan->field) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    bool refused;
    status = ama_program_list_open(program, owner, owner, plan->field, service, &refused);
    if (status.code != AMA_OK) { return status; }
    if (service->incarnation != cursor->incarnation) {
        status = ama_program_list_close(service);
        return status.code == AMA_OK ? ama_status_make(AMA_STALE_REFERENCE, NULL, 0) : status;
    }
    return ama_status_success();
}
