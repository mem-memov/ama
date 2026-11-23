/** @file
 * @brief Compose protected-name selection and verified private-root creation before compiler request files exist.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires precise ownership on every failed prefix, not pathname-based cleanup.
 */
#define _XOPEN_SOURCE 700
#include "internal.h"
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
SourceWorkspaceStatus source_workspace_open(SourceWorkspace *workspace, const char *parent,
    const char *const *protected_paths, size_t count, SourceTreeControl control) {
    if (!workspace || workspace->attempted) { return SOURCE_WORKSPACE_INVALID; }
    workspace->attempted = true;
    if (!parent || !*parent || (count && !protected_paths)) {
        return source_workspace_fail(workspace, SOURCE_WORKSPACE_INVALID, parent, 0, SOURCE_MATERIAL_TREE_OK);
    }
    if (count > SIZE_MAX / sizeof(*protected_paths)) { return source_workspace_fail(workspace, SOURCE_WORKSPACE_LIMIT, parent, 0, SOURCE_MATERIAL_TREE_OK); }
    for (size_t i = 0; i < count; i++) {
        if (!protected_paths[i] || protected_paths[i][0] != '/') { return source_workspace_fail(workspace, SOURCE_WORKSPACE_INVALID, protected_paths[i], 0, SOURCE_MATERIAL_TREE_OK); }
    }
    workspace->control = control;
    SourceWorkspaceStatus status = source_workspace_parent(workspace, parent);
    if (status != SOURCE_WORKSPACE_OK) { return status; }
    status = source_workspace_choose(workspace, protected_paths, count);
    if (status != SOURCE_WORKSPACE_OK) { return status; }
    workspace->root_entry = (SourceCatalogImageEntry){.path = workspace->path,
        .metadata = {.st_dev = 0, .st_ino = 1, .st_mode = S_IFDIR | 0700}, .metadata_captured = true, .children_complete = true};
    if (source_material_tree_prepare(&workspace->root, workspace->parent, workspace->name,
        workspace->root_entries, 1, control) != SOURCE_MATERIAL_TREE_OK) {
        return source_workspace_fail(workspace, source_workspace_tree_status(&workspace->root), workspace->path,
            workspace->root.system_error, workspace->root.status);
    }
    int directory = openat(workspace->parent, workspace->name, O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
    if (directory < 0) { return source_workspace_fail(workspace, SOURCE_WORKSPACE_IO, workspace->path, errno, SOURCE_MATERIAL_TREE_OK); }
    SourceWorkspaceIssue issue;
    status = source_workspace_root(workspace, directory, &issue);
    if (status != SOURCE_WORKSPACE_OK) {
        (void)close(directory); return source_workspace_fail(workspace, status, issue.path, issue.system_error, issue.tree_status);
    }
    if (!source_material_tree_verify(&workspace->root)) {
        (void)close(directory);
        return source_workspace_fail(workspace, source_workspace_tree_status(&workspace->root), workspace->root.failed_path,
            workspace->root.system_error, workspace->root.status);
    }
    workspace->directory = directory; workspace->ready = true; return SOURCE_WORKSPACE_OK;
}
