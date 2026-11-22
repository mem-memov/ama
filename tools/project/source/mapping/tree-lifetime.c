/** @file
 * @brief Own explicit selected-tree placement strings independently of their captured bytes.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) retains failed
 * preparation prefixes; [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * keeps physical translation distinct from nominal identity generation.
 */
#include <string.h>
#include "tree.h"
void source_tree_placement_init(SourceTreePlacement *placement, SourceAllocator allocator) {
    memset(placement, 0, sizeof(*placement)); source_arena_init(&placement->arena, allocator);
}
void source_tree_placement_destroy(SourceTreePlacement *placement) {
    source_arena_destroy(&placement->arena); memset(placement, 0, sizeof(*placement));
}
