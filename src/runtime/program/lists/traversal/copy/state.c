/** @file
 * @brief Copy fixed cursor and binding values after destination lists are complete.
 * [CONT-04](../../../../../../spec/language/continuations.md#cont-04) preserves
 * numerical boundaries without applying construction-time insertion corrections.
 * No busy grant, pending call frame or native stack pointer is copied.
 */
#include "internal.h"

AmaStatus ama_foreach_control_copy(const AmaObjectControlCopy *copy, void *context) {
    const AmaProgramType *type = NULL;
    AmaStatus status = ama_foreach_copy_type(context, copy->world, copy->source.type_id, &type);
    if (status.code != AMA_OK) { return status; }
    const unsigned char *source = copy->source_storage;
    unsigned char *destination = copy->destination_storage;
    for (size_t index = 0; index < type->foreach_count; index++) {
        const AmaProgramForeachPlan *plan = &type->foreach_scopes[index].plan;
        const AmaProgramForeachCursor *original = (const void *)(source + plan->cursor_offset);
        AmaProgramForeachCursor *cursor = (void *)(destination + plan->cursor_offset);
        if (original->active) {
            const AmaHandleList *list = NULL;
            status = ama_foreach_copy_list(type, destination, plan->field, &list);
            if (status.code != AMA_OK) { return status; }
            *cursor = *original;
            cursor->owner = copy->destination;
            cursor->incarnation = list->incarnation;
            status = ama_foreach_copy_membership(copy, type, original->current, &cursor->current);
            if (status.code != AMA_OK) { return status; }
        }
        const AmaProgramForeachBinding *old_binding = (const void *)(source + plan->binding_offset);
        AmaProgramForeachBinding *binding = (void *)(destination + plan->binding_offset);
        /* Shared cells may be assigned repeatedly from the same immutable source.
         * These are value copies, never increments or additional target credits. */
        *binding = *old_binding;
        if (binding->associated) {
            status = ama_foreach_copy_membership(copy, type, old_binding->origin, &binding->origin);
            if (status.code != AMA_OK) { return status; }
        }
    }
    return ama_status_success();
}
