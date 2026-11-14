/** @file
 * @brief Reject removed iteration origins before target access or owning copying.
 * [FLOW-05](../../../../../spec/language/lists.md#flow-05) preserves a membership
 * association on the binding while independent handle copies have no origin.
 * [LIST-10](../../../../../spec/language/lists.md#list-10) permits only exact
 * method-direct authority to inspect a currently busy source collection.
 */
#include "internal.h"

/** Mark only checked absence as expected refusal; malformed metadata is technical. */
static AmaStatus ama_foreach_absence(AmaStatus status, bool *refused) {
    *refused = status.code == AMA_EMPTY_REFERENCE || status.code == AMA_STALE_REFERENCE;
    return status;
}

AmaStatus ama_program_foreach_origin(AmaProgram *program, AmaMembership origin, AmaHandle target,
    const AmaProgramListService *authority, bool *refused) {
    *refused = false;
    AmaPoolCell *cell = NULL;
    AmaStatus status = ama_program_cell(program, origin.owner, &cell);
    if (status.code != AMA_OK) { return ama_foreach_absence(status, refused); }
    AmaObjectType *type = ama_world_type(program->world, origin.owner.type_id);
    if (origin.field >= type->field_count) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    const AmaObjectField *field = &type->fields[origin.field];
    if (field->kind != AMA_FIELD_OWNING_LIST && field->kind != AMA_FIELD_REFERENCE_LIST) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    const AmaHandleList *list = (const void *)((const unsigned char *)ama_pool_payload(&type->pool, cell) + field->offset);
    if (list->acquisition != NULL) {
        status = ama_program_list_authorize(authority, program, origin.owner, origin.field, list);
        if (status.code != AMA_OK) { *refused = status.code == AMA_BUSY; return status; }
    }
    /* This complete identity read has no callback, conversion or allocation.
     * The context has one executor, so no list mutation can interleave here. */
    const AmaMembershipRecord *record = NULL;
    status = ama_membership_lookup(list, origin, &record);
    if (status.code != AMA_OK) { return ama_foreach_absence(status, refused); }
    if (!ama_handle_equal(record->target, target)) {
        *refused = true;
        return ama_status_make(AMA_STALE_REFERENCE, NULL, 0);
    }
    if (target.generation == 0) { return ama_foreach_absence(ama_status_make(AMA_EMPTY_REFERENCE, NULL, 0), refused); }
    if (target.type_id != field->type_id) { return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0); }
    return ama_foreach_absence(ama_program_cell(program, target, &cell), refused);
}

AmaStatus ama_program_foreach_guard(AmaProgram *program, const AmaProgramValue *value,
    const AmaProgramListService *authority, bool *refused) {
    *refused = false;
    const AmaProgramForeachBinding *binding = value->foreach_binding;
    if (binding == NULL || !binding->associated) { return ama_status_success(); }
    if (binding->roles == 0 || value->location == NULL || !ama_handle_equal(*value->location, binding->installed)
        || !ama_handle_equal(value->handle, binding->installed)) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    return ama_program_foreach_origin(program, binding->origin, binding->installed, authority, refused);
}

void ama_program_foreach_detach(AmaProgramValue *value, AmaHandle replacement) {
    AmaProgramForeachBinding *binding = value->foreach_binding;
    if (binding != NULL && !ama_handle_equal(*value->location, replacement)) {
        binding->associated = false;
        binding->origin = (AmaMembership){0};
        binding->installed = (AmaHandle){0};
    }
}
