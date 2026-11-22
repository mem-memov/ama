/** @file
 * @brief Retire delivery membership and claim each scheduled active identity once.
 * [PKG-08](../../../../../spec/project/packages.md#pkg-08) bounds expansions by
 * distinct active identities; [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03)
 * gives all complete/failed records one operation lifetime.
 */
#include "roles.h"
void acquisition_roles_init(AcquisitionRoles *roles, SourceAllocator allocator) {
    *roles = (AcquisitionRoles){0};
    source_arena_init(&roles->arena, allocator);
}
void acquisition_roles_destroy(AcquisitionRoles *roles) {
    source_arena_destroy(&roles->arena);
    *roles = (AcquisitionRoles){0};
}
const AcquisitionRoleRecord *acquisition_roles_take(AcquisitionRoles *roles) {
    AcquisitionRoleRecord *record = roles->pending;
    if (!record) { return NULL; }
    roles->pending = record->pending_next;
    if (!roles->pending) { roles->last_pending = NULL; }
    record->pending_next = NULL;
    record->expanded = true;
    return record;
}
