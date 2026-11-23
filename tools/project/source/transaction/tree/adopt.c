/** @file
 * @brief Reopen only exact journal-owned filesystem classes without creating or rewriting them.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires repeatable recovery that never mistakes an unrelated replacement for saved material.
 */
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include "internal.h"
/** Cleanup can recognize only the exact permission transition made by our cleanup. */
static bool mode_matches(mode_t actual, mode_t expected, SourceTreeAdoption admission) {
    return actual == expected || (admission == SOURCE_TREE_ADOPT_CLEANUP && S_ISDIR(expected) && actual == (expected | 0700));
}
/** Retain a descriptor even when identity admission fails; teardown never unlinks it. */
static bool reopen(SourceMaterialTree *tree, size_t index, SourceTreeAdoption admission) {
    SourceMaterialRow *row = &tree->rows[index];
    const SourceCatalogImageEntry *entry = row->expected;
    if (!source_material_tree_permit(tree, SOURCE_TREE_OPEN, entry->path)) { return false; }
    if (admission == SOURCE_TREE_ADOPT_CLEANUP && index && tree->rows[row->parent].removed) {
        row->created = row->removed = true;
        return true;
    }
    struct stat named;
    if (fstatat(source_material_tree_parent(tree, index), row->name, &named, AT_SYMLINK_NOFOLLOW) != 0) {
        if (admission == SOURCE_TREE_ADOPT_CLEANUP && errno == ENOENT) {
            row->created = row->removed = true;
            return true;
        }
        return source_material_tree_fail(tree, SOURCE_MATERIAL_TREE_IO, entry->path, errno);
    }
    if (named.st_dev != entry->metadata.st_dev || named.st_ino != entry->metadata.st_ino
        || !mode_matches(named.st_mode, entry->metadata.st_mode, admission)) {
        return source_material_tree_fail(tree, SOURCE_MATERIAL_TREE_CHANGED, entry->path, 0);
    }
    int flags = O_RDONLY | O_NOFOLLOW | O_CLOEXEC | O_NONBLOCK;
    if (S_ISDIR(entry->metadata.st_mode)) { flags |= O_DIRECTORY; }
    row->descriptor = openat(source_material_tree_parent(tree, index), row->name, flags);
    if (row->descriptor < 0 || fstat(row->descriptor, &row->identity) != 0) {
        return source_material_tree_fail(tree, SOURCE_MATERIAL_TREE_IO, entry->path, errno);
    }
    if (row->identity.st_dev != entry->metadata.st_dev || row->identity.st_ino != entry->metadata.st_ino
        || !mode_matches(row->identity.st_mode, entry->metadata.st_mode, admission)) {
        return source_material_tree_fail(tree, SOURCE_MATERIAL_TREE_CHANGED, entry->path, 0);
    }
    row->created = true;
    row->identified = true;
    row->written = S_ISREG(entry->metadata.st_mode) ? source_length(entry->content) : 0;
    return true;
}
SourceMaterialTreeStatus source_material_tree_adopt(SourceMaterialTree *tree, int parent, const char *name,
    const SourceCatalogImageEntry *const *entries, size_t count, SourceTreeAdoption admission, SourceTreeControl control) {
    if (!tree || tree->attempted || parent < 0 || (admission != SOURCE_TREE_ADOPT_EXACT && admission != SOURCE_TREE_ADOPT_CLEANUP)) { return SOURCE_MATERIAL_TREE_INVALID; }
    tree->attempted = true;
    tree->parent = parent;
    tree->name = name;
    tree->control = control;
    if (!source_material_tree_shape(tree, entries, count)) { return tree->status; }
    for (size_t i = 0; i < count; i++) {
        if (!reopen(tree, i, admission)) { return tree->status; }
    }
    if (!source_material_tree_permit(tree, SOURCE_TREE_VERIFY, name) || !source_material_tree_verify(tree)) { return tree->status; }
    tree->ready = admission == SOURCE_TREE_ADOPT_EXACT;
    return SOURCE_MATERIAL_TREE_OK;
}
