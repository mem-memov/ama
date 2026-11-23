/** @file
 * @brief Retain finite root-setting proposals and precise failed prefixes until final proof consumption.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03)
 * keeps all arena-owned proposal keys/rows alive with their borrower.
 */
#include "internal.h"
#include <string.h>
void project_source_root_settings_init(ProjectSourceRootSettings *owner, SourceAllocator allocator) {
    memset(owner, 0, sizeof(*owner)); source_arena_init(&owner->arena, allocator);
}
void project_source_root_settings_destroy(ProjectSourceRootSettings *owner) {
    source_arena_destroy(&owner->arena); memset(owner, 0, sizeof(*owner));
}
