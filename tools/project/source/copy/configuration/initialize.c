/** @file
 * @brief Prepare permissions for a proven wholly absent destination version.
 * [TOOL-01](../../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * initializes an empty environment; [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * permits only actual explicit extension without synthesizing an original scope.
 */
#include "prepare.h"
#include <string.h>
/** An exact missing key is proven only by the closest observed complete
 * directory ancestor. Existing, unknown or regular ancestors grant no repair. */
static bool version_absent(const SourceCatalogImage *image, const char *path) {
    size_t length = strlen(path);
    for (;;) {
        for (size_t i = 0; i < image->count; i++) {
            const SourceCatalogImageEntry *entry = &image->entries[i];
            if (strlen(entry->path) == length && !memcmp(entry->path, path, length)) {
                return length != strlen(path) && entry->metadata_captured
                    && S_ISDIR(entry->metadata.st_mode) && entry->children_complete;
            }
        }
        if (length <= 1) { return false; }
        while (length && path[length - 1] != '/') { length--; }
        if (length > 1) { length--; }
    }
}
SourceConfigurationStatus source_copy_configuration_initialize(SourceArena *arena, SourceTextEdits *edits,
    const SourceCopyClosure *copy, const SourceCloneMap *map, const SourceObservedImage *original,
    const SourcePackageEnvironment *final, const SourcePackageScope *scope,
    SourceSelectionRoots *output, SourceConfigurationIssue *issue) {
    if (!issue) { return SOURCE_CONFIGURATION_INVALID; } *issue = (SourceConfigurationIssue){0};
    if (!arena || !edits || edits->attempted || edits->status != SOURCE_EDITS_OK || !output
        || !copy || !copy->valid || !map || !map->ready || map->capture != copy->tree->capture
        || !original || !original->valid || !original->sealed || original->original != copy->tree->capture
        || !final || !final->valid) { return SOURCE_CONFIGURATION_INVALID; }
    bool owned = false;
    for (const SourcePackageScope *row = final->first; row; row = row->next) { if (row == scope) { owned = true; } }
    if (!owned || edits->original != scope->document.source || source_length(edits->original)
        || !version_absent(&original->image, scope->version->physical_path)) { return SOURCE_CONFIGURATION_INVALID; }
    SourceSelectionRoots baseline = {0}, additions = {0};
    issue->permission_status = source_copy_permission_additions(arena, copy, map, final, scope,
        &baseline, &additions, &issue->permission);
    if (issue->permission_status != SOURCE_PERMISSION_OK) { return SOURCE_CONFIGURATION_PERMISSIONS; }
    issue->edits_status = source_copy_reexpress_selectors(edits, &scope->document, final->view, additions.entries, additions.count);
    if (issue->edits_status != SOURCE_EDITS_OK) { return SOURCE_CONFIGURATION_EDITS; }
    *output = additions; return SOURCE_CONFIGURATION_OK;
}
