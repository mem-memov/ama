/** @file
 * @brief Keep proposed clone identities alive without owning original module storage.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) retains failed
 * preparation prefixes; [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * separates owned clone names from original nominal identities.
 */
#include <string.h>
#include "map.h"
void source_clone_map_init(SourceCloneMap *map, SourceAllocator allocator, const SourceRootCapture *capture) {
    memset(map, 0, sizeof(*map));
    source_arena_init(&map->arena, allocator);
    map->capture = capture;
}
void source_clone_map_destroy(SourceCloneMap *map) {
    source_arena_destroy(&map->arena);
    memset(map, 0, sizeof(*map));
}
