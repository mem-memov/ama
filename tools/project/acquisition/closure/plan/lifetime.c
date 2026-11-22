/** @file
 * @brief Own and retire complete/failed delivery fixed-point metadata.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) requires
 * reverse teardown before borrowed source/fact owners expire.
 */
#include "internal.h"
void acquisition_closure_init(AcquisitionClosure *closure, SourceAllocator allocator) {
    *closure = (AcquisitionClosure){0}; source_arena_init(&closure->arena, allocator);
    acquisition_roles_init(&closure->roles, allocator); acquisition_members_init(&closure->members, allocator);
}
void acquisition_closure_destroy(AcquisitionClosure *closure) {
    acquisition_members_destroy(&closure->members); acquisition_roles_destroy(&closure->roles);
    source_arena_destroy(&closure->arena); *closure = (AcquisitionClosure){0};
}
AcquisitionClosureStatus acquisition_closure_fail(AcquisitionClosure *closure, AcquisitionClosureStatus status, const char *message) {
    closure->status = status;
    if (!closure->issue.message) { closure->issue.message = message; }
    return status;
}
