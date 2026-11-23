/** @file
 * @brief Materialize each immutable compiler request once through the existing private-tree byte owner.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires complete exact request bytes and no source mutation; successful
 * serialization alone does not prove safe filesystem ownership or final semantics.
 */
#define _XOPEN_SOURCE 700
#include "internal.h"
SourceWorkspaceStatus source_workspace_snapshot(SourceWorkspace *workspace, SourceWorkspaceSlot slot,
    const DeliverySourceSnapshotView *view) {
    if (!workspace || !workspace->ready || workspace->cleaned || slot < SOURCE_WORKSPACE_ORIGINAL
        || slot >= SOURCE_WORKSPACE_FILE_COUNT || !view || workspace->files[slot].attempted) { return SOURCE_WORKSPACE_INVALID; }
    SourceWorkspaceStatus status = source_workspace_verify(workspace);
    if (status != SOURCE_WORKSPACE_OK) { return status; }
    SourceWorkspaceFile *file = &workspace->files[slot]; file->attempted = true;
    file->entry.path = source_arena_join(&workspace->arena, workspace->path, "/", file->name);
    if (!file->entry.path) { return source_workspace_fail(workspace, SOURCE_WORKSPACE_MEMORY, workspace->path, 0, SOURCE_MATERIAL_TREE_OK); }
    status = source_workspace_encode(workspace, file->entry.path, view, &file->entry.content);
    if (status != SOURCE_WORKSPACE_OK) { return status; }
    file->entry.metadata = (struct stat){.st_dev = 0, .st_ino = (ino_t)(slot + 2), .st_mode = S_IFREG | 0600};
    file->entry.metadata_captured = true;
    /* The root's standalone empty-tree proof is no longer the current joined
     * membership proof. Retain its inode ownership while the workspace verifies
     * the independently owned child prefixes; never forge standalone readiness. */
    workspace->root.ready = false;
    if (source_material_tree_prepare(&file->tree, workspace->directory, file->name, file->entries, 1,
        workspace->control) != SOURCE_MATERIAL_TREE_OK) {
        return source_workspace_fail(workspace, source_workspace_tree_status(&file->tree), file->entry.path,
            file->tree.system_error, file->tree.status);
    }
    return source_workspace_verify(workspace);
}
