/** @file
 * @brief Retain the exact explicit scratch-parent descriptor after neutral physical traversal.
 * [SRC-02](../../../../spec/language/source-and-names.md#src-02)
 * rejects aliases and wrong kinds; [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * does not grant semantic-root authority from an opened observation parent.
 */
#include "internal.h"
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
SourceWorkspaceStatus source_workspace_parent(SourceWorkspace *workspace, const char *path) {
    SourcePhysicalStatus status = source_physical_resolve(&workspace->arena, path, SOURCE_PHYSICAL_DIRECTORY,
        false, &workspace->parent_path, &workspace->physical_issue);
    if (status != SOURCE_PHYSICAL_OK) {
        return source_workspace_fail(workspace, status == SOURCE_PHYSICAL_STORAGE ? SOURCE_WORKSPACE_MEMORY : SOURCE_WORKSPACE_PHYSICAL,
            workspace->physical_issue.path, 0, SOURCE_MATERIAL_TREE_OK);
    }
    workspace->parent = open(workspace->parent_path.path, O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
    if (workspace->parent < 0) { return source_workspace_fail(workspace, SOURCE_WORKSPACE_IO, workspace->parent_path.path, errno, SOURCE_MATERIAL_TREE_OK); }
    struct stat actual;
    if (fstat(workspace->parent, &actual) != 0) { return source_workspace_fail(workspace, SOURCE_WORKSPACE_IO, workspace->parent_path.path, errno, SOURCE_MATERIAL_TREE_OK); }
    const struct stat *expected = &workspace->parent_path.metadata;
    if (actual.st_dev != expected->st_dev || actual.st_ino != expected->st_ino || actual.st_mode != expected->st_mode) {
        return source_workspace_fail(workspace, SOURCE_WORKSPACE_CHANGED, workspace->parent_path.path, 0, SOURCE_MATERIAL_TREE_OK);
    }
    return SOURCE_WORKSPACE_OK;
}
