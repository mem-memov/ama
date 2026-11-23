/** @file
 * @brief Prevalidate every private tree component and parent before creating anything.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * does not authorize traversal or partial material through malformed saved data.
 */
#include <stdint.h>
#include <string.h>
#include "internal.h"
/** Component strings never traverse, alias a parent, or contain a separator. */
static bool component(const char *name) {
    return name && *name && !strchr(name, '/') && strcmp(name, ".") && strcmp(name, "..");
}
bool source_material_tree_shape(SourceMaterialTree *tree, const SourceCatalogImageEntry *const *entries, size_t count) {
    if (!component(tree->name) || !entries || !count || count > SIZE_MAX / sizeof(SourceMaterialRow)) {
        return source_material_tree_fail(tree, SOURCE_MATERIAL_TREE_INVALID, tree->name, 0);
    }
    SourceMaterialRow *rows = source_arena_allocate(&tree->arena, count * sizeof(*rows));
    if (!rows) { return source_material_tree_fail(tree, SOURCE_MATERIAL_TREE_MEMORY, tree->name, 0); }
    for (size_t i = 0; i < count; i++) { rows[i] = (SourceMaterialRow){.descriptor = -1}; }
    tree->rows = rows;
    tree->count = count;
    for (size_t i = 0; i < count; i++) {
        const SourceCatalogImageEntry *entry = entries[i];
        if (!entry || !entry->path || !entry->metadata_captured ||
            (!S_ISDIR(entry->metadata.st_mode) && !S_ISREG(entry->metadata.st_mode)) ||
            (S_ISDIR(entry->metadata.st_mode) ? !entry->children_complete : !entry->content)) {
            return source_material_tree_fail(tree, SOURCE_MATERIAL_TREE_INVALID, entry ? entry->path : tree->name, 0);
        }
        rows[i].expected = entry;
        if (!i) { rows[i].name = tree->name; continue; }
        const char *slash = strrchr(entry->path, '/');
        if (!slash || !component(slash + 1) || !source_image_below(entry->path, entries[0]->path)) {
            return source_material_tree_fail(tree, SOURCE_MATERIAL_TREE_INVALID, entry->path, 0);
        }
        rows[i].name = slash + 1;
        size_t length = (size_t)(slash - entry->path), parent = 0;
        for (; parent < i; parent++) {
            if (strlen(rows[parent].expected->path) == length && !memcmp(rows[parent].expected->path, entry->path, length)) { break; }
        }
        if (parent == i || !S_ISDIR(rows[parent].expected->metadata.st_mode)) {
            return source_material_tree_fail(tree, SOURCE_MATERIAL_TREE_INVALID, entry->path, 0);
        }
        for (size_t j = 0; j < i; j++) {
            if (!strcmp(rows[j].expected->path, entry->path)) { return source_material_tree_fail(tree, SOURCE_MATERIAL_TREE_INVALID, entry->path, 0); }
        }
        rows[i].parent = parent;
    }
    return true;
}
