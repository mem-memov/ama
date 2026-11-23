/** @file
 * @brief Own closed write-query nodes without taking over immutable producer bytes.
 * [ARCH-03](../../../../../../spec/implementation/compiler.md#arch-03)
 * retains failed prefixes and expires all borrowed outputs at explicit teardown.
 */
#include "join.h"
#include <string.h>
void source_operation_writes_init(SourceOperationWrites *writes, SourceAllocator allocator) {
    memset(writes, 0, sizeof(*writes)); source_arena_init(&writes->arena, allocator);
}
void source_operation_writes_destroy(SourceOperationWrites *writes) {
    source_arena_destroy(&writes->arena); memset(writes, 0, sizeof(*writes));
}
