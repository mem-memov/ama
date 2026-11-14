/** @file
 * @brief Reject obsolete active collections before continuation constructors run.
 * [CONT-04](../../../../../../spec/language/continuations.md#cont-04) validates
 * every active source collection before preparing a copy. Removed memberships
 * remain copyable; only collection replacement, invalid boundaries or unavailable
 * acquisition make this preflight fail.
 */
#include "internal.h"

AmaStatus ama_foreach_control_validate(AmaWorld *world, AmaHandle source, const void *storage,
    const AmaServiceState *service, void *context) {
    const AmaProgramType *type = NULL;
    AmaStatus status = ama_foreach_copy_type(context, world, source.type_id, &type);
    if (status.code != AMA_OK) { return status; }
    const unsigned char *base = storage;
    for (size_t index = 0; index < type->foreach_count; index++) {
        const AmaProgramForeachPlan *plan = &type->foreach_scopes[index].plan;
        const AmaProgramForeachCursor *cursor = (const void *)(base + plan->cursor_offset);
        if (!cursor->active) { continue; }
        const AmaHandleList *list = NULL;
        status = ama_foreach_copy_list(type, storage, plan->field, &list);
        if (status.code != AMA_OK) { return status; }
        if (!ama_handle_equal(cursor->owner, source) || cursor->field != plan->field
            || !ama_handle_equal(list->owner, source) || list->field != plan->field || list->incarnation == 0
            || !service->resume.valid || !cursor->role_active) {
            return ama_status_make(AMA_INVALID_STATE, NULL, 0);
        }
        if (cursor->incarnation != list->incarnation) { return ama_status_make(AMA_STALE_REFERENCE, NULL, 0); }
        if (list->acquisition != NULL) { return ama_status_make(AMA_BUSY, NULL, 0); }
        if (cursor->boundary > list->count) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
        if (!ama_handle_equal(cursor->current.owner, source) || cursor->current.field != cursor->field
            || cursor->current.incarnation != cursor->incarnation) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    }
    return ama_status_success();
}
