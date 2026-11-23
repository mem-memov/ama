/** @file
 * @brief Decode actual plus-storage parent components without counting ordinary folders.
 * [SRC-05](../../../../../../spec/language/source-and-names.md#src-05)
 * separates physical plus anchors from ordinary directories;
 * [TOOL-01](../../../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * requires occupied module-parent positions to count nearest first.
 */
#include "internal.h"
#include <string.h>
/** Own an exact physical stem; there is no fixed path-length limit. */
static const char *prefix(SourceArena *arena, const char *path, size_t length) {
    char *copy = source_arena_allocate(arena, length + 1);
    if (copy) { memcpy(copy, path, length); copy[length] = 0; } return copy;
}
SourceCopyParentStatus source_copy_parent_scan(const SourceCopyParentInputs *inputs,
    const SourceCloneMapping *row, SourceCopyParentIssue *issue) {
    SourcePlacedIdentity child;
    issue->requester = row; issue->path = row->path;
    issue->identity_status = source_placement_identity(&inputs->mains->arena, inputs->roots, inputs->root_count,
        row->path, MODULE_PATH_MODULE, &child);
    if (issue->identity_status != SOURCE_IDENTITY_OK) {
        return issue->identity_status == SOURCE_IDENTITY_MEMORY ? SOURCE_COPY_PARENT_MEMORY : SOURCE_COPY_PARENT_IDENTITY;
    }
    if (strcmp(row->identity, child.identity)) { return SOURCE_COPY_PARENT_IDENTITY; }
    size_t boundary = strlen(inputs->roots[child.root].path), position = 0;
    for (size_t cursor = strlen(row->path); cursor > boundary; cursor--) {
        if (cursor < 2 || row->path[cursor - 1] != '/' || row->path[cursor - 2] != '+' || cursor - 2 <= boundary) { continue; }
        const char *path = prefix(&inputs->mains->arena, row->path, cursor - 2);
        if (!path) { return SOURCE_COPY_PARENT_MEMORY; } issue->path = path;
        SourcePlacedIdentity parent;
        issue->identity_status = source_placement_identity(&inputs->mains->arena, inputs->roots, inputs->root_count,
            path, MODULE_PATH_MODULE, &parent);
        if (issue->identity_status != SOURCE_IDENTITY_OK) {
            return issue->identity_status == SOURCE_IDENTITY_MEMORY ? SOURCE_COPY_PARENT_MEMORY : SOURCE_COPY_PARENT_IDENTITY;
        }
        SourceCopyParentStatus status = source_copy_parent_provider(inputs, row, path, parent.identity, position++, issue);
        if (status != SOURCE_COPY_PARENT_OK) { return status; }
    }
    return SOURCE_COPY_PARENT_OK;
}
