/** @file
 * @brief Serialize exact neutral snapshot data into owned immutable bytes before creating a request file.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04)
 * preserves exact captured data; [PROJECT-01](../../../../spec/implementation/project-tools.md#project-01)
 * allows preparation-side serialization without importing compiler execution state.
 */
#include "internal.h"
#include "delivery/source/snapshot/write.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
SourceWorkspaceStatus source_workspace_encode(SourceWorkspace *workspace, const char *path,
    const DeliverySourceSnapshotView *view, const SourceText **output) {
    char *bytes = NULL; size_t length = 0;
    FILE *stream = open_memstream(&bytes, &length);
    if (!stream) { return source_workspace_fail(workspace, SOURCE_WORKSPACE_IO, path, errno, SOURCE_MATERIAL_TREE_OK); }
    errno = 0; bool encoded = delivery_source_snapshot_write(stream, view);
    int error = ferror(stream) ? errno : 0;
    int closed = fclose(stream); if (closed && !error) { error = errno; }
    if (!encoded || closed) {
        free(bytes); return source_workspace_fail(workspace, SOURCE_WORKSPACE_SNAPSHOT, path, error, SOURCE_MATERIAL_TREE_OK);
    }
    workspace->text_status = source_manager_add(&workspace->texts, path, (const unsigned char *)bytes, length, output);
    free(bytes);
    if (workspace->text_status != SOURCE_OK) {
        SourceWorkspaceStatus status = workspace->text_status == SOURCE_OUT_OF_MEMORY ? SOURCE_WORKSPACE_MEMORY
            : workspace->text_status == SOURCE_LIMIT ? SOURCE_WORKSPACE_LIMIT : SOURCE_WORKSPACE_SNAPSHOT;
        return source_workspace_fail(workspace, status, path, 0, SOURCE_MATERIAL_TREE_OK);
    }
    return SOURCE_WORKSPACE_OK;
}
