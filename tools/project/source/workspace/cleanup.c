/** @file
 * @brief Clean only verified joined workspace ownership while preserving primary and retained-data failures.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * forbids recursive deletion of unknown private names and requires explicit
 * retained locations when cleanup cannot prove safe removal.
 */
#include "internal.h"
/** Record cleanup independently; a retry never erases the original preparation issue. */
static SourceWorkspaceStatus tree_failure(SourceWorkspace *workspace, const SourceMaterialTree *tree) {
    workspace->cleanup_status = source_workspace_tree_status(tree);
    workspace->cleanup_issue = (SourceWorkspaceIssue){tree->failed_path, tree->system_error, tree->status};
    return workspace->cleanup_status;
}
SourceWorkspaceStatus source_workspace_cleanup(SourceWorkspace *workspace) {
    if (!workspace) { return SOURCE_WORKSPACE_INVALID; }
    if (workspace->cleaned) { return SOURCE_WORKSPACE_OK; }
    workspace->cleanup_status = SOURCE_WORKSPACE_OK; workspace->cleanup_issue = (SourceWorkspaceIssue){0};
    if (workspace->root.attempted) {
        if (workspace->directory >= 0) {
            workspace->cleanup_status = source_workspace_check(workspace, &workspace->cleanup_issue);
            if (workspace->cleanup_status != SOURCE_WORKSPACE_OK) { return workspace->cleanup_status; }
            for (size_t i = SOURCE_WORKSPACE_FILE_COUNT; i; i--) {
                SourceMaterialTree *tree = &workspace->files[i-1].tree;
                if (tree->attempted && !source_material_tree_cleanup(tree)) { return tree_failure(workspace, tree); }
            }
        }
        if (!source_material_tree_cleanup(&workspace->root)) { return tree_failure(workspace, &workspace->root); }
    }
    workspace->ready = false; workspace->cleaned = true; return SOURCE_WORKSPACE_OK;
}
