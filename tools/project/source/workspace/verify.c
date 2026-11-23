/** @file
 * @brief Verify joined workspace-root identity, owned request bytes and complete immediate membership.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * does not authorize deleting or using a replaced private object merely because
 * its path/name or contents look familiar.
 */
#include "internal.h"
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
SourceWorkspaceStatus source_workspace_root(const SourceWorkspace *workspace, int directory, SourceWorkspaceIssue *issue) {
    *issue = (SourceWorkspaceIssue){.path = workspace->path};
    if (workspace->parent < 0 || directory < 0 || !workspace->path) { return SOURCE_WORKSPACE_INVALID; }
    struct stat opened, named;
    if (fstat(directory, &opened) != 0 || fstatat(workspace->parent, workspace->name, &named, AT_SYMLINK_NOFOLLOW) != 0) {
        issue->system_error = errno; return SOURCE_WORKSPACE_IO;
    }
    if (!source_material_tree_owns(&workspace->root, workspace->root_entry.path, &opened)
        || !source_material_tree_owns(&workspace->root, workspace->root_entry.path, &named)) { return SOURCE_WORKSPACE_CHANGED; }
    *issue = (SourceWorkspaceIssue){0}; return SOURCE_WORKSPACE_OK;
}
/** A slot name alone is insufficient: even failed prefixes need actual retained
 * identified creation ownership before this root can treat a member as known. */
static bool owned(const SourceWorkspace *workspace, const char *name, const struct stat *actual) {
    for (size_t i = 0; i < SOURCE_WORKSPACE_FILE_COUNT; i++) {
        const SourceWorkspaceFile *file = &workspace->files[i];
        if (!strcmp(name, file->name) && file->entry.path && source_material_tree_owns(&file->tree, file->entry.path, actual)) { return true; }
    }
    return false;
}
/** Retain foreign-name witnesses before readdir/closedir storage expires. */
static SourceWorkspaceStatus membership(SourceWorkspace *workspace, SourceWorkspaceIssue *issue) {
    int descriptor = fcntl(workspace->directory, F_DUPFD_CLOEXEC, 0);
    if (descriptor < 0) { issue->system_error = errno; return SOURCE_WORKSPACE_IO; }
    DIR *directory = fdopendir(descriptor);
    if (!directory) { issue->system_error = errno; (void)close(descriptor); return SOURCE_WORKSPACE_IO; }
    rewinddir(directory); SourceWorkspaceStatus status = SOURCE_WORKSPACE_OK;
    for (;;) {
        errno = 0; struct dirent *entry = readdir(directory);
        if (!entry) { if (errno) { issue->system_error = errno; status = SOURCE_WORKSPACE_IO; } break; }
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) { continue; }
        struct stat actual;
        if (fstatat(workspace->directory, entry->d_name, &actual, AT_SYMLINK_NOFOLLOW) != 0) { issue->system_error = errno; status = SOURCE_WORKSPACE_IO; break; }
        if (!owned(workspace, entry->d_name, &actual)) {
            const char *path = source_arena_join(&workspace->arena, workspace->path, "/", entry->d_name);
            if (path) { issue->path = path; status = SOURCE_WORKSPACE_CHANGED; }
            else { status = SOURCE_WORKSPACE_MEMORY; }
            break;
        }
    }
    if (closedir(directory) != 0 && status == SOURCE_WORKSPACE_OK) { issue->system_error = errno; status = SOURCE_WORKSPACE_IO; }
    return status;
}
SourceWorkspaceStatus source_workspace_check(SourceWorkspace *workspace, SourceWorkspaceIssue *issue) {
    SourceWorkspaceStatus status = source_workspace_root(workspace, workspace->directory, issue);
    if (status != SOURCE_WORKSPACE_OK) { return status; }
    issue->path = workspace->path;
    for (size_t i = 0; i < SOURCE_WORKSPACE_FILE_COUNT; i++) {
        SourceMaterialTree *tree = &workspace->files[i].tree;
        if (tree->attempted && !source_material_tree_verify(tree)) {
            *issue = (SourceWorkspaceIssue){tree->failed_path, tree->system_error, tree->status}; return source_workspace_tree_status(tree);
        }
    }
    status = membership(workspace, issue);
    if (status == SOURCE_WORKSPACE_OK) { *issue = (SourceWorkspaceIssue){0}; }
    return status;
}
SourceWorkspaceStatus source_workspace_verify(SourceWorkspace *workspace) {
    if (!workspace || !workspace->ready || workspace->cleaned) { return SOURCE_WORKSPACE_INVALID; }
    SourceWorkspaceIssue issue; SourceWorkspaceStatus status = source_workspace_check(workspace, &issue);
    return status == SOURCE_WORKSPACE_OK ? status : source_workspace_fail(workspace, status, issue.path, issue.system_error, issue.tree_status);
}
