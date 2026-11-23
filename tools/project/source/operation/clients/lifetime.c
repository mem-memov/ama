/** @file
 * @brief Retire client-setting reports without owning external configuration or compiler facts.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03)
 * gives every failed allocation prefix one explicit owner.
 */
#include "clients.h"
#include <string.h>
void source_operation_clients_init(SourceOperationClients *owner, SourceAllocator allocator) {
    memset(owner, 0, sizeof(*owner)); source_arena_init(&owner->arena, allocator);
}
void source_operation_clients_destroy(SourceOperationClients *owner) {
    source_arena_destroy(&owner->arena); memset(owner, 0, sizeof(*owner));
}
