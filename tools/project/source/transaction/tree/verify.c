/** @file
 * @brief Verify exact private bytes, retained inode ownership and complete child membership.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * prevents a replaced entry or extra child from being deleted during cleanup.
 */
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "internal.h"
/** Read through the retained exclusive descriptor, including an incomplete write prefix. */
static bool bytes(SourceMaterialTree *tree, SourceMaterialRow *row, const struct stat *actual) {
    if (actual->st_size < 0 || (uintmax_t)actual->st_size != row->written) {
        return source_material_tree_fail(tree, SOURCE_MATERIAL_TREE_CHANGED, row->expected->path, 0);
    }
    unsigned char buffer[8192];
    size_t used = 0;
    while (used < row->written) {
        size_t count = row->written - used;
        if (count > sizeof(buffer)) { count = sizeof(buffer); }
        off_t offset = (off_t)used;
        if (offset < 0 || (uintmax_t)offset != used) {
            return source_material_tree_fail(tree, SOURCE_MATERIAL_TREE_IO, row->expected->path, EOVERFLOW);
        }
        ssize_t received = pread(row->descriptor, buffer, count, offset);
        if (received < 0 && errno == EINTR) { continue; }
        if (received < 0) { return source_material_tree_fail(tree, SOURCE_MATERIAL_TREE_IO, row->expected->path, errno); }
        if (!received || memcmp(buffer, source_bytes(row->expected->content) + used, (size_t)received)) {
            return source_material_tree_fail(tree, SOURCE_MATERIAL_TREE_CHANGED, row->expected->path, 0);
        }
        used += (size_t)received;
    }
    return true;
}
/** Only created, still-owned direct children are permitted in a private directory. */
static bool children(SourceMaterialTree *tree, size_t index) {
    int descriptor = dup(tree->rows[index].descriptor);
    if (descriptor < 0) { return source_material_tree_fail(tree, SOURCE_MATERIAL_TREE_IO, tree->rows[index].expected->path, errno); }
    DIR *directory = fdopendir(descriptor);
    if (!directory) {
        int error = errno;
        (void)close(descriptor);
        return source_material_tree_fail(tree, SOURCE_MATERIAL_TREE_IO, tree->rows[index].expected->path, error);
    }
    rewinddir(directory);
    bool complete = true;
    for (;;) {
        errno = 0;
        struct dirent *entry = readdir(directory);
        if (!entry) {
            if (errno) { complete = source_material_tree_fail(tree, SOURCE_MATERIAL_TREE_IO, tree->rows[index].expected->path, errno); }
            break;
        }
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) { continue; }
        bool owned = false;
        for (size_t i = 1; i < tree->count; i++) {
            SourceMaterialRow *row = &tree->rows[i];
            if (row->created && !row->removed && row->parent == index && !strcmp(row->name, entry->d_name)) { owned = true; break; }
        }
        if (!owned) { complete = source_material_tree_fail(tree, SOURCE_MATERIAL_TREE_CHANGED, tree->rows[index].expected->path, 0); break; }
    }
    if (closedir(directory) != 0 && complete) {
        complete = source_material_tree_fail(tree, SOURCE_MATERIAL_TREE_IO, tree->rows[index].expected->path, errno);
    }
    return complete;
}
bool source_material_tree_verify(SourceMaterialTree *tree) {
    if (!tree || !tree->attempted || (tree->count && !tree->rows)) { return false; }
    for (size_t i = 0; i < tree->count; i++) {
        SourceMaterialRow *row = &tree->rows[i];
        if (!row->created || row->removed) { continue; }
        if (!row->identified || row->descriptor < 0) {
            return source_material_tree_fail(tree, SOURCE_MATERIAL_TREE_CHANGED, row->expected->path, 0);
        }
        struct stat actual, named;
        if (fstat(row->descriptor, &actual) != 0 ||
            fstatat(source_material_tree_parent(tree, i), row->name, &named, AT_SYMLINK_NOFOLLOW) != 0) {
            return source_material_tree_fail(tree, SOURCE_MATERIAL_TREE_IO, row->expected->path, errno);
        }
        if (actual.st_dev != row->identity.st_dev || actual.st_ino != row->identity.st_ino || actual.st_mode != row->identity.st_mode
            || named.st_dev != actual.st_dev || named.st_ino != actual.st_ino || named.st_mode != actual.st_mode) {
            return source_material_tree_fail(tree, SOURCE_MATERIAL_TREE_CHANGED, row->expected->path, 0);
        }
        if (S_ISDIR(actual.st_mode) ? !children(tree, i) : !bytes(tree, row, &actual)) { return false; }
    }
    return true;
}
