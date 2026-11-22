/** @file
 * @brief Own additional deletion material without owning the original compiler or raw tree.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) retains
 * every failed prefix until deterministic reverse teardown.
 */
#include <string.h>
#include "selection.h"
void source_removal_selection_init(SourceRemovalSelection *selection, SourceAllocator allocator) {
    memset(selection, 0, sizeof(*selection));
    source_arena_init(&selection->arena, allocator);
    source_manager_init(&selection->files, allocator);
}
void source_removal_selection_destroy(SourceRemovalSelection *selection) {
    source_arena_destroy(&selection->arena);
    source_manager_destroy(&selection->files);
    memset(selection, 0, sizeof(*selection));
}
