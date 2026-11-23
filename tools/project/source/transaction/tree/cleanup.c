/** @file
 * @brief Remove only a fully verified private creation prefix, leaving foreign entries intact.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires reporting retained data instead of recursive deletion by pathname.
 */
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include "internal.h"
bool source_material_tree_cleanup(SourceMaterialTree *tree) {
    if (!source_material_tree_verify(tree)) { return false; }
    tree->ready = false;
    /* Cleanup may need owner traversal rights on our own readonly directories.
     * Change only verified owned descriptors and retain the updated mode on failure. */
    for (size_t i = 0; i < tree->count; i++) {
        SourceMaterialRow *row = &tree->rows[i];
        if (!row->created || row->removed || !S_ISDIR(row->identity.st_mode)) { continue; }
        mode_t mode = row->identity.st_mode | 0700;
        if (fchmod(row->descriptor, mode & 07777) != 0) {
            return source_material_tree_fail(tree, SOURCE_MATERIAL_TREE_IO, row->expected->path, errno);
        }
        row->identity.st_mode = mode;
    }
    for (size_t i = tree->count; i; i--) {
        SourceMaterialRow *row = &tree->rows[i - 1];
        if (!row->created || row->removed) { continue; }
        int flags = S_ISDIR(row->identity.st_mode) ? AT_REMOVEDIR : 0;
        if (unlinkat(source_material_tree_parent(tree, i - 1), row->name, flags) != 0) {
            return source_material_tree_fail(tree, SOURCE_MATERIAL_TREE_IO, row->expected->path, errno);
        }
        row->removed = true;
    }
    if (tree->parent >= 0 && fsync(tree->parent) != 0) {
        return source_material_tree_fail(tree, SOURCE_MATERIAL_TREE_IO, tree->name, errno);
    }
    tree->ready = false;
    return true;
}
