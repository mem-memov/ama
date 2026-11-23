/** @file
 * @brief Admit a structurally explicit finite image before installing its read-only adapter.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * distinguishes captured absence from unavailable inputs; [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * requires one exact prepared material set rather than silent path/content merging.
 */
#include "internal.h"
#include <string.h>
/** Only physical envelope keys terminate ancestor provenance; semantic roots
 * inside a wider envelope must retain their actual parent chain. */
static bool root_entry(const SourceCatalogImage *image, const char *path) {
    for (size_t i = 0; i < source_image_bound_count(image); i++) {
        if (!strcmp(source_image_bound_path(image, i), path)) { return true; }
    }
    return false;
}
/** Validate row structure independently of whether common selection will read it. */
static bool entry_shape(const SourceCatalogImageEntry *entry) {
    if (!entry->path || entry->path[0] != '/') { return false; }
    size_t length = strlen(entry->path);
    bool root_prefix = true;
    for (size_t i = 0; i < length; i++) { root_prefix &= entry->path[i] == '/'; }
    if (!root_prefix && entry->path[length - 1] == '/') { return false; }
    if (!entry->metadata_captured) { return !entry->content && !entry->children_complete; }
    return (!entry->content || S_ISREG(entry->metadata.st_mode)) &&
        (!entry->children_complete || S_ISDIR(entry->metadata.st_mode));
}
SourceImageStatus source_catalog_image_prepare(SourceCatalogImage *output,
    const SourceCatalogImageEntry *entries, size_t count, const ModuleRoot *roots,
    size_t root_count, SourceImageBounds bounds, SourceImageIssue *issue) {
    if (!issue) { return SOURCE_IMAGE_INVALID; }
    *issue = (SourceImageIssue){0};
    if (!output || (count && !entries) || (root_count && !roots) || (bounds.count && !bounds.paths)) { return SOURCE_IMAGE_INVALID; }
    SourceCatalogImage image = {.entries = entries, .count = count, .roots = roots, .root_count = root_count, .bounds = bounds};
    for (size_t i = 0; i < count; i++) {
        issue->entry = i; issue->path = entries[i].path;
        if (!entry_shape(&entries[i])) { return SOURCE_IMAGE_INVALID; }
        for (size_t j = 0; j < i; j++) { if (!strcmp(entries[i].path, entries[j].path)) { return SOURCE_IMAGE_DUPLICATE; } }
    }
    for (size_t i = 0; i < root_count; i++) {
        *issue = (SourceImageIssue){.path = roots[i].path};
        if (!roots[i].path || roots[i].path[0] != '/' ||
            (roots[i].excluded_count && !roots[i].excluded) ||
            (roots[i].kind != MODULE_ROOT_PROJECT && roots[i].kind != MODULE_ROOT_STANDARD)) { return SOURCE_IMAGE_INVALID; }
        for (size_t j = 0; j < roots[i].excluded_count; j++) { if (!roots[i].excluded[j]) { return SOURCE_IMAGE_INVALID; } }
        const SourceCatalogImageEntry *root = source_image_find(&image, roots[i].path, strlen(roots[i].path));
        if (!root || !root->metadata_captured || !S_ISDIR(root->metadata.st_mode)) { issue->path = roots[i].path; return SOURCE_IMAGE_PARENT; }
    }
    for (size_t i = 0; i < source_image_bound_count(&image); i++) {
        const char *path = source_image_bound_path(&image, i);
        *issue = (SourceImageIssue){.path = path};
        if (!path || path[0] != '/') { return SOURCE_IMAGE_INVALID; }
        const SourceCatalogImageEntry *row = source_image_find(&image, path, strlen(path));
        if (!row || !row->metadata_captured || !S_ISDIR(row->metadata.st_mode)) { return SOURCE_IMAGE_PARENT; }
    }
    for (size_t i = 0; i < count; i++) {
        issue->entry = i; issue->path = entries[i].path;
        bool covered = false;
        for (size_t j = 0; j < source_image_bound_count(&image); j++) {
            covered |= source_image_below(entries[i].path, source_image_bound_path(&image, j));
        }
        if (!covered) { return SOURCE_IMAGE_OUTSIDE; }
        if (root_entry(&image, entries[i].path)) { continue; }
        const char *slash = strrchr(entries[i].path, '/');
        size_t length = slash == entries[i].path ? 1 : (size_t)(slash - entries[i].path);
        const SourceCatalogImageEntry *parent = source_image_find(&image, entries[i].path, length);
        if (!parent || (parent->metadata_captured && !S_ISDIR(parent->metadata.st_mode))) { return SOURCE_IMAGE_PARENT; }
    }
    image.reader = (ModuleCatalogReader){.tree = {.context = output, .root = source_image_root,
        .child = source_image_child, .entries = source_image_entries}, .read = source_image_read};
    image.valid = true; *output = image; *issue = (SourceImageIssue){0};
    return SOURCE_IMAGE_OK;
}
