/** @file
 * @brief Reuse actual parent providers or demand the one corresponding original ancestor.
 * [TOOL-01](../../../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * forbids skipping occupied positions or selecting an alternative ancestor;
 * [TOOL-02](../../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * preserves explicit/active placements and injectivity independently of context.
 */
#include "internal.h"
#include <string.h>
/** Explicit raw units and active roles have independently fixed placements. */
static bool independent(const SourceCopyClosure *copy, const ModuleSource *source) {
    for (size_t i = 0; i < copy->tree->count; i++) { if (copy->tree->units[i]->source == source) { return true; } }
    for (const AcquisitionRoleRecord *role = copy->closure.roles.records; role; role = role->next) {
        if (!strcmp(role->identity, source->logical_path)) { return role->role == ACQUISITION_ACTIVE; }
    }
    return false;
}
/** Exact original removal membership never means that a physical file was absent. */
static bool removed(const SourceRemovalSelection *cut, const char *path) {
    if (cut) { for (size_t i = 0; i < cut->path_count; i++) { if (!strcmp(cut->paths[i], path)) { return true; } } }
    return false;
}
/** Match only a genuine structural-context role; missing active placements are
 * not permission to silently choose a different destination for active code. */
static bool context(const SourceCopyClosure *copy, const ModuleSource *source) {
    for (const AcquisitionRoleRecord *role = copy->closure.roles.records; role; role = role->next) {
        if (!strcmp(role->identity, source->logical_path)) { return role->role == ACQUISITION_CONTEXT; }
    }
    return false;
}
SourceCopyParentStatus source_copy_parent_provider(const SourceCopyParentInputs *inputs, const SourceCloneMapping *requester,
    const char *path, const char *identity, size_t position, SourceCopyParentIssue *issue) {
    const SourceCapturedUnit *ancestor = source_copy_context_ancestor(inputs->mains->capture->units, requester->original, position);
    issue->required = ancestor ? ancestor->source : NULL;
    for (const SourceCloneMapping *row = inputs->mains->first; row; row = row->next) {
        bool same_path = !strcmp(path, row->path), same_name = !strcmp(identity, row->identity);
        if (!same_path && !same_name) { continue; }
        if (!same_path || !same_name) { return SOURCE_COPY_PARENT_CONFLICT; }
        if (independent(inputs->copy, row->original)) { return SOURCE_COPY_PARENT_OK; }
        return ancestor && row->original == ancestor->source ? SOURCE_COPY_PARENT_OK : SOURCE_COPY_PARENT_CONFLICT;
    }
    bool absence = source_image_path_absent(&inputs->observed->image, path);
    if (!absence && !removed(inputs->removal, path)) {
        for (size_t i = 0; i < inputs->observed->image.count; i++) {
            const SourceCatalogImageEntry *entry = &inputs->observed->image.entries[i];
            if (strcmp(entry->path, path)) { continue; }
            if (!entry->metadata_captured) { return SOURCE_COPY_PARENT_UNOBSERVED; }
            if (!S_ISREG(entry->metadata.st_mode)) { return SOURCE_COPY_PARENT_KIND; }
            return entry->content ? SOURCE_COPY_PARENT_OK : SOURCE_COPY_PARENT_UNOBSERVED;
        }
        return SOURCE_COPY_PARENT_UNOBSERVED;
    }
    if (!ancestor) { return SOURCE_COPY_PARENT_MISSING; }
    if (!context(inputs->copy, ancestor->source)) { return SOURCE_COPY_PARENT_CONFLICT; }
    const SourceCloneMapping *added = NULL;
    issue->map_status = source_clone_map_add(inputs->mains, ancestor->source, path, identity, &added);
    return issue->map_status == SOURCE_CLONE_OK ? SOURCE_COPY_PARENT_OK
        : issue->map_status == SOURCE_CLONE_MEMORY ? SOURCE_COPY_PARENT_MEMORY : SOURCE_COPY_PARENT_MAP;
}
