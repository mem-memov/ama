/** @file
 * @brief Own raw selected tree bytes and projection metadata through planning.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) preserves
 * deterministic teardown of complete and failed captures.
 */
#include <string.h>
#include "tree.h"
void source_tree_selection_init(SourceTreeSelection *selection, SourceAllocator allocator) {
    memset(selection, 0, sizeof(*selection));
    source_arena_init(&selection->arena, allocator);
    source_manager_init(&selection->files, allocator);
}
void source_tree_selection_destroy(SourceTreeSelection *selection) {
    source_arena_destroy(&selection->arena);
    source_manager_destroy(&selection->files);
    memset(selection, 0, sizeof(*selection));
}
