/** @file
 * @brief Retain verified actual staging inode classes for durable ownership records.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * cannot recover a private copy using the compiler image's projected identities.
 */
#include <stdint.h>
#include "internal.h"
bool source_material_tree_snapshot(SourceMaterialTree *tree, SourceArena *arena, const SourceCatalogImageEntry **output) {
    if (!tree || !tree->ready || !arena || !output || !source_material_tree_verify(tree)) { return false; }
    if (tree->count > SIZE_MAX / sizeof(SourceCatalogImageEntry)) {
        return source_material_tree_fail(tree, SOURCE_MATERIAL_TREE_MEMORY, tree->name, 0);
    }
    SourceCatalogImageEntry *rows = source_arena_allocate(arena, tree->count * sizeof(*rows));
    if (!rows) { return source_material_tree_fail(tree, SOURCE_MATERIAL_TREE_MEMORY, tree->name, 0); }
    for (size_t i = 0; i < tree->count; i++) {
        rows[i] = *tree->rows[i].expected;
        rows[i].metadata = tree->rows[i].identity;
    }
    *output = rows;
    return true;
}
