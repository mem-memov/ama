/** @file
 * @brief Retire rebuild intention metadata independently of borrowed source and generated outputs.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03)
 * assigns failed prefixes one explicit owner with no hidden cleanup effects.
 */
#include "rebuild.h"
#include <string.h>
void source_operation_rebuild_init(SourceOperationRebuild *owner, SourceAllocator allocator) {
    memset(owner, 0, sizeof(*owner)); source_arena_init(&owner->arena, allocator);
}
void source_operation_rebuild_destroy(SourceOperationRebuild *owner) {
    source_arena_destroy(&owner->arena); memset(owner, 0, sizeof(*owner));
}
