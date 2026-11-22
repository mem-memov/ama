/** @file
 * @brief Resolve and capture one explicit original module file or directory operand.
 * [TOOL-01](../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * preserves exact filesystem placement; [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * uses the original catalog rather than guessing source identity from a basename.
 */
#include <string.h>
#include "internal.h"
/** A regular-file SOURCE must be one exact original selected main, never a companion. */
static const SourceCapturedUnit *find_original_unit(const SourceRootCapture *capture, const char *path) {
    for (const SourceCapturedUnit *unit = capture->units; unit; unit = unit->next) {
        if (!strcmp(path, source_path(unit->source->text))) { return unit; }
    }
    return NULL;
}
SourceTreeStatus source_tree_selection_read(SourceTreeSelection *selection,
    const SourceRootCapture *capture, const char *source) {
    if (!selection || selection->attempted || !capture || !capture->valid || !source || !*source) { return SOURCE_TREE_INVALID; }
    selection->attempted = true;
    selection->capture = capture;
    if (source_physical_resolve(&selection->arena, source, SOURCE_PHYSICAL_ENDPOINT, false,
        &selection->operand, &selection->physical_issue) != SOURCE_PHYSICAL_OK) {
        selection->failed_path = selection->physical_issue.path;
        return selection->status = SOURCE_TREE_PHYSICAL;
    }
    const char *path = selection->operand.path;
    bool inside = false;
    for (size_t index = 0; index < capture->root_count; index++) {
        if (source_tree_inside(path, capture->roots[index].path)) { inside = true; break; }
    }
    if (!inside) { return selection->status = SOURCE_TREE_OUTSIDE; }
    if (S_ISDIR(selection->operand.metadata.st_mode)) {
        selection->file_status = acquisition_directory_capture_root(&selection->arena, &selection->files,
            &selection->operand, &selection->members, &selection->failed_path);
    } else {
        const SourceCapturedUnit *unit = find_original_unit(capture, path);
        if (!unit) { return selection->status = SOURCE_TREE_NOT_MODULE; }
        if (selection->operand.metadata.st_dev != unit->source->device || selection->operand.metadata.st_ino != unit->source->inode) {
            return selection->status = SOURCE_TREE_CHANGED;
        }
        const char *slash = strrchr(path, '/');
        char *parent = source_arena_copy(&selection->arena, path);
        if (!parent) { return selection->status = SOURCE_TREE_MEMORY; }
        parent[slash - path] = 0;
        selection->file_status = acquisition_unit_capture(&selection->arena, &selection->files,
            *parent ? parent : "/", slash + 1, ACQUISITION_ACTIVE, &selection->members, &selection->failed_path);
    }
    if (selection->file_status != SOURCE_OK) { return selection->status = SOURCE_TREE_FILES; }
    selection->status = source_tree_project(selection, capture);
    selection->valid = selection->status == SOURCE_TREE_OK;
    return selection->status;
}
