/** @file
 * @brief Own all complete-operation children in reverse dependency order after every prefix.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03)
 * retains failed diagnostics without borrowing destroyed images or producer bytes.
 */
#include "complete.h"
#include <string.h>
void source_operation_complete_init(SourceOperationComplete *owner, SourceAllocator allocator) {
    memset(owner, 0, sizeof(*owner)); source_arena_init(&owner->arena, allocator);
    source_operation_writes_init(&owner->writes, allocator);
    source_package_environment_init(&owner->original_environment, allocator);
    source_operation_plan_init(&owner->plan, allocator); source_operation_material_init(&owner->material, allocator);
    source_operation_roots_init(&owner->roots, allocator); source_operation_clients_init(&owner->clients, allocator);
    source_operation_rebuild_init(&owner->rebuild, allocator);
}
void source_operation_complete_destroy(SourceOperationComplete *owner) {
    source_operation_rebuild_destroy(&owner->rebuild); source_operation_clients_destroy(&owner->clients);
    source_operation_roots_destroy(&owner->roots); source_operation_material_destroy(&owner->material);
    source_operation_plan_destroy(&owner->plan); source_arena_destroy(&owner->arena);
    source_package_environment_destroy(&owner->original_environment); source_operation_writes_destroy(&owner->writes);
    memset(owner, 0, sizeof(*owner));
}
