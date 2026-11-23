/** @file
 * @brief Exclusively create private tree entries and retain their exact written prefixes.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * stages bytes before source replacement and retains ownership on every failure.
 */
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include "internal.h"
bool source_material_tree_create(SourceMaterialTree *tree, size_t index) {
    SourceMaterialRow *row = &tree->rows[index];
    const SourceCatalogImageEntry *entry = row->expected;
    int parent = source_material_tree_parent(tree, index);
    if (!source_material_tree_permit(tree, SOURCE_TREE_CREATE, entry->path)) { return false; }
    bool directory = S_ISDIR(entry->metadata.st_mode);
    if (directory) {
        if (mkdirat(parent, row->name, 0700) != 0) { return source_material_tree_fail(tree, SOURCE_MATERIAL_TREE_IO, entry->path, errno); }
        row->created = true;
    }
    int flags = directory ? O_RDONLY | O_DIRECTORY : O_RDWR | O_CREAT | O_EXCL;
    row->descriptor = openat(parent, row->name, flags | O_NOFOLLOW | O_CLOEXEC, 0600);
    if (row->descriptor < 0) { return source_material_tree_fail(tree, SOURCE_MATERIAL_TREE_IO, entry->path, errno); }
    row->created = true;
    if (fstat(row->descriptor, &row->identity) != 0) { return source_material_tree_fail(tree, SOURCE_MATERIAL_TREE_IO, entry->path, errno); }
    row->identified = true;
    if (directory) { return true; }
    size_t length = source_length(entry->content);
    while (row->written < length) {
        if (!source_material_tree_permit(tree, SOURCE_TREE_WRITE, entry->path)) { return false; }
        size_t remaining = length - row->written;
        if (remaining > 65536) { remaining = 65536; }
        ssize_t written = write(row->descriptor, source_bytes(entry->content) + row->written, remaining);
        if (written < 0 && errno == EINTR) { continue; }
        if (written <= 0) { return source_material_tree_fail(tree, SOURCE_MATERIAL_TREE_IO, entry->path, written < 0 ? errno : EIO); }
        row->written += (size_t)written;
    }
    return true;
}
bool source_material_tree_finish(SourceMaterialTree *tree) {
    for (size_t i = tree->count; i; i--) {
        SourceMaterialRow *row = &tree->rows[i - 1];
        const char *path = row->expected->path;
        if (!source_material_tree_permit(tree, SOURCE_TREE_MODE, path)) { return false; }
        if (fchmod(row->descriptor, row->expected->metadata.st_mode & 07777) != 0) {
            return source_material_tree_fail(tree, SOURCE_MATERIAL_TREE_IO, path, errno);
        }
        row->identity.st_mode = row->expected->metadata.st_mode;
        if (!source_material_tree_permit(tree, SOURCE_TREE_SYNC, path)) { return false; }
        if (fsync(row->descriptor) != 0) { return source_material_tree_fail(tree, SOURCE_MATERIAL_TREE_IO, path, errno); }
    }
    if (fsync(tree->parent) != 0) { return source_material_tree_fail(tree, SOURCE_MATERIAL_TREE_IO, tree->name, errno); }
    return true;
}
