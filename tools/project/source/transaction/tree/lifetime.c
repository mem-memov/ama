/** @file
 * @brief Retire private-tree descriptors without implicitly deleting recovery data.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires retained material to remain reportable when cleanup cannot complete.
 */
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "internal.h"
void source_material_tree_init(SourceMaterialTree *tree, SourceAllocator allocator) {
    memset(tree, 0, sizeof(*tree));
    tree->parent = -1;
    source_arena_init(&tree->arena, allocator);
}
int source_material_tree_destroy(SourceMaterialTree *tree) {
    int first = 0;
    for (size_t i = tree->count; i; i--) {
        if (tree->rows[i - 1].descriptor >= 0 && close(tree->rows[i - 1].descriptor) != 0 && !first) { first = errno; }
    }
    source_arena_destroy(&tree->arena);
    memset(tree, 0, sizeof(*tree));
    tree->parent = -1;
    return first;
}
bool source_material_tree_fail(SourceMaterialTree *tree, SourceMaterialTreeStatus status, const char *path, int error) {
    tree->ready = false;
    tree->status = status;
    tree->failed_path = path;
    tree->system_error = error;
    return false;
}
bool source_material_tree_permit(SourceMaterialTree *tree, SourceTreeOperation operation, const char *path) {
    return !tree->control.permit || tree->control.permit(tree->control.context, operation, path)
        || source_material_tree_fail(tree, SOURCE_MATERIAL_TREE_CANCELLED, path, 0);
}
int source_material_tree_parent(const SourceMaterialTree *tree, size_t index) {
    return index ? tree->rows[tree->rows[index].parent].descriptor : tree->parent;
}
