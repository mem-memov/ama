/** @file
 * @brief Retire root-setting report/path scratch without owning source snapshots or client settings.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03)
 * retains failed prefixes until explicit owner destruction.
 */
#include "roots.h"
#include <string.h>
void source_operation_roots_init(SourceOperationRoots *owner, SourceAllocator allocator) {
    memset(owner, 0, sizeof(*owner)); source_arena_init(&owner->arena, allocator);
}
void source_operation_roots_destroy(SourceOperationRoots *owner) {
    source_arena_destroy(&owner->arena); memset(owner, 0, sizeof(*owner));
}
