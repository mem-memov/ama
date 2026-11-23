/** @file
 * @brief Choose an exact exclusive private name outside every protected source/target prefix.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * keeps private preparation apart from source mutations; no random collision
 * assumption or directory-name namespace inference supplies permission.
 */
#include "internal.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
SourceWorkspaceStatus source_workspace_choose(SourceWorkspace *workspace, const char *const *protected_paths, size_t count) {
    const char *parent = workspace->parent_path.path;
    for (size_t i = 0; i < count; i++) {
        if (source_image_below(parent, protected_paths[i])) {
            return source_workspace_fail(workspace, SOURCE_WORKSPACE_BOUNDARY, parent, 0, SOURCE_MATERIAL_TREE_OK);
        }
    }
    size_t length = strlen(parent);
    if (length > SIZE_MAX - sizeof(workspace->name) - 2) { return source_workspace_fail(workspace, SOURCE_WORKSPACE_LIMIT, parent, 0, SOURCE_MATERIAL_TREE_OK); }
    workspace->path = source_arena_allocate(&workspace->arena, length + sizeof(workspace->name) + 2);
    if (!workspace->path) { return source_workspace_fail(workspace, SOURCE_WORKSPACE_MEMORY, parent, 0, SOURCE_MATERIAL_TREE_OK); }
    for (size_t sequence = 0;; sequence++) {
        int written = snprintf(workspace->name, sizeof(workspace->name), ".ama-source-%ju-%zu", (uintmax_t)getpid(), sequence);
        if (written < 0 || (size_t)written >= sizeof(workspace->name)) { return source_workspace_fail(workspace, SOURCE_WORKSPACE_LIMIT, parent, 0, SOURCE_MATERIAL_TREE_OK); }
        (void)sprintf(workspace->path, "%s%s%s", parent, !strcmp(parent, "/") ? "" : "/", workspace->name);
        bool forbidden = false;
        for (size_t i = 0; i < count; i++) {
            if (source_image_below(workspace->path, protected_paths[i]) || source_image_below(protected_paths[i], workspace->path)) { forbidden = true; break; }
        }
        if (!forbidden) {
            struct stat actual;
            if (fstatat(workspace->parent, workspace->name, &actual, AT_SYMLINK_NOFOLLOW) != 0) {
                if (errno == ENOENT) { return SOURCE_WORKSPACE_OK; }
                return source_workspace_fail(workspace, SOURCE_WORKSPACE_IO, workspace->path, errno, SOURCE_MATERIAL_TREE_OK);
            }
        }
        if (sequence == SIZE_MAX) { return source_workspace_fail(workspace, SOURCE_WORKSPACE_LIMIT, parent, 0, SOURCE_MATERIAL_TREE_OK); }
    }
}
