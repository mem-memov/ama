/** @file
 * @brief Retire workspace request children before their borrowed root descriptors and byte owners.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03)
 * keeps every failed creation prefix owned; [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * forbids implicit deletion of unverified retained data.
 */
#include "internal.h"
#include <errno.h>
#include <string.h>
#include <unistd.h>
void source_workspace_init(SourceWorkspace *workspace, SourceAllocator allocator) {
    memset(workspace, 0, sizeof(*workspace)); workspace->parent = workspace->directory = -1;
    source_arena_init(&workspace->arena, allocator); source_manager_init(&workspace->texts, allocator);
    source_material_tree_init(&workspace->root, allocator); workspace->root_entries[0] = &workspace->root_entry;
    const char *names[] = {"original.json", "final.json"};
    for (size_t i = 0; i < SOURCE_WORKSPACE_FILE_COUNT; i++) {
        workspace->files[i].name = names[i]; workspace->files[i].entries[0] = &workspace->files[i].entry;
        source_material_tree_init(&workspace->files[i].tree, allocator);
    }
}
int source_workspace_destroy(SourceWorkspace *workspace) {
    int first = 0;
    for (size_t i = SOURCE_WORKSPACE_FILE_COUNT; i; i--) {
        int error = source_material_tree_destroy(&workspace->files[i-1].tree); if (!first) { first = error; }
    }
    if (workspace->directory >= 0 && close(workspace->directory) != 0 && !first) { first = errno; }
    int error = source_material_tree_destroy(&workspace->root); if (!first) { first = error; }
    if (workspace->parent >= 0 && close(workspace->parent) != 0 && !first) { first = errno; }
    source_manager_destroy(&workspace->texts); source_arena_destroy(&workspace->arena);
    memset(workspace, 0, sizeof(*workspace)); workspace->parent = workspace->directory = -1; return first;
}
SourceWorkspaceStatus source_workspace_fail(SourceWorkspace *workspace, SourceWorkspaceStatus status,
    const char *path, int error, SourceMaterialTreeStatus tree_status) {
    if (workspace->status == SOURCE_WORKSPACE_OK) {
        workspace->status = status; workspace->issue = (SourceWorkspaceIssue){path, error, tree_status};
    }
    workspace->ready = false; return workspace->status;
}
SourceWorkspaceStatus source_workspace_tree_status(const SourceMaterialTree *tree) {
    return tree->status == SOURCE_MATERIAL_TREE_MEMORY ? SOURCE_WORKSPACE_MEMORY
        : tree->status == SOURCE_MATERIAL_TREE_CHANGED ? SOURCE_WORKSPACE_CHANGED : SOURCE_WORKSPACE_TREE;
}
