/** @file
 * @brief Retire physical clone metadata independently of borrowed captured bytes.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03)
 * keeps complete and refused preparations under one explicit lifetime.
 */
#include "files.h"
void source_file_map_init(SourceFileMap *map, SourceAllocator allocator) {
    *map = (SourceFileMap){0}; source_arena_init(&map->arena, allocator);
}
void source_file_map_destroy(SourceFileMap *map) {
    source_arena_destroy(&map->arena); *map = (SourceFileMap){0};
}
