/** @file
 * @brief Reserve conditional foreach reference cells without constructing values or referents.
 * [NAME-06](../../../../spec/language/declarations.md#name-06) gives each introduced
 * binding lexical identity; [FLOW-05](../../../../spec/language/lists.md#flow-05)
 * reuses an existing compatible cell rather than saving/restoring its previous handle.
 */
#include "internal.h"
bool plan_storage_cells(ProgramPlan *plan, PlannedType *type) {
    for (const SemanticForeachBinding *role = type->type->foreach_bindings; role != NULL; role = role->next) {
        ReferenceOriginStorage *origin = type->origins;
        while (origin != NULL && origin->symbol != role->binding) { origin = origin->next; }
        if (origin == NULL) {
            origin = plan_allocate(plan, sizeof(*origin));
            if (origin == NULL) { return false; }
            *origin = (ReferenceOriginStorage){.symbol = role->binding, .slot = type->origin_count++};
            if (type->last_origin == NULL) { type->origins = origin; }
            else { type->last_origin->next = origin; }
            type->last_origin = origin;
        }
        if (!role->introduced) { continue; }
        ScopedStorageCell *cell = plan_allocate(plan, sizeof(*cell));
        if (cell == NULL) { return false; }
        *cell = (ScopedStorageCell){.symbol = role->binding, .introduction = role, .slot = type->cell_count++};
        if (type->last_cell == NULL) { type->cells = cell; }
        else { type->last_cell->next = cell; }
        type->last_cell = cell;
    }
    return true;
}
