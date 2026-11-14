/** @file
 * @brief Remap nonowning control references through complete scalar/forest maps.
 * [CONT-04](../../../../../../spec/language/continuations.md#cont-04) applies
 * COPY-04 to local handles and installed/current targets. Already mapped internal
 * destinations are outside the disjoint source key set, making repeated shared
 * binding visits idempotent and allowing a later complete-forest map to finish
 * cross-element references without recopying namespace or boundary state.
 */
#include "internal.h"

AmaStatus ama_foreach_control_references(AmaWorld *world, uint64_t identity, void *storage,
    AmaObjectReferenceMap map, const void *map_context, void *context) {
    const AmaProgramType *type = NULL;
    AmaStatus status = ama_foreach_copy_type(context, world, identity, &type);
    if (status.code != AMA_OK) { return status; }
    unsigned char *base = storage;
    for (size_t index = 0; index < type->foreach_count; index++) {
        const AmaProgramForeachPlan *plan = &type->foreach_scopes[index].plan;
        AmaProgramForeachCursor *cursor = (void *)(base + plan->cursor_offset);
        AmaProgramForeachBinding *binding = (void *)(base + plan->binding_offset);
        if (cursor->active) { cursor->target = map(map_context, cursor->target); }
        if (binding->roles != 0 && plan->local) { binding->local_value = map(map_context, binding->local_value); }
        if (binding->associated) { binding->installed = map(map_context, binding->installed); }
    }
    return ama_status_success();
}
