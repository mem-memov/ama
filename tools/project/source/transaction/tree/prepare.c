/** @file
 * @brief Publish private-tree readiness only after complete durable write and exact readback.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * does not accept exclusive creation alone as verified recovery material.
 */
#include "internal.h"
SourceMaterialTreeStatus source_material_tree_prepare(SourceMaterialTree *tree, int parent, const char *name,
    const SourceCatalogImageEntry *const *entries, size_t count, SourceTreeControl control) {
    if (!tree || tree->attempted || parent < 0) { return SOURCE_MATERIAL_TREE_INVALID; }
    tree->attempted = true;
    tree->parent = parent;
    tree->name = name;
    tree->control = control;
    if (!source_material_tree_shape(tree, entries, count)) { return tree->status; }
    for (size_t i = 0; i < count; i++) {
        if (!source_material_tree_create(tree, i)) { return tree->status; }
    }
    if (!source_material_tree_finish(tree) || !source_material_tree_permit(tree, SOURCE_TREE_VERIFY, name)
        || !source_material_tree_verify(tree)) { return tree->status; }
    tree->ready = true;
    return SOURCE_MATERIAL_TREE_OK;
}
