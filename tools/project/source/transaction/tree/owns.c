/** @file
 * @brief Match actual private inode observations to retained creation ownership without granting content authority.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * preserves foreign/replaced entries; a byte-equal name or planned symbolic
 * class is never a substitute for an identified, still-owned created inode.
 */
#include "internal.h"
#include <string.h>
bool source_material_tree_owns(const SourceMaterialTree *tree, const char *path, const struct stat *actual) {
    if (!tree || !tree->attempted || !path || !actual || (tree->count && !tree->rows)) { return false; }
    for (size_t i = 0; i < tree->count; i++) {
        const SourceMaterialRow *row = &tree->rows[i];
        if (row->created && row->identified && !row->removed && row->descriptor >= 0
            && row->expected && row->expected->path && !strcmp(row->expected->path, path)
            && row->identity.st_dev == actual->st_dev && row->identity.st_ino == actual->st_ino
            && row->identity.st_mode == actual->st_mode) { return true; }
    }
    return false;
}
