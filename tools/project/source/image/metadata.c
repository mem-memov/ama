/** @file
 * @brief Probe exact image entries and ancestor directories without treating unknowns as missing.
 * [SRC-02](../../../../spec/language/source-and-names.md#src-02) requires exact
 * directory/alias provenance; [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * distinguishes complete observed absence from unavailable metadata.
 */
#include "internal.h"
#include <string.h>
/** Retain the exact requested catalog-owned key for every typed read refusal. */
static ModuleStatus fail(ModuleCatalog *catalog, ModuleStatus status, const char *path) {
    catalog->issue = (ModuleIssue){.status = status, .path = path}; return status;
}
/** A missing row proves absence only in an explicitly complete parent listing. */
static ModuleStatus lookup(const SourceCatalogImage *image, ModuleCatalog *catalog,
    const char *path, size_t length, struct stat *output) {
    const SourceCatalogImageEntry *entry = source_image_find(image, path, length);
    if (!entry) {
        size_t parent_length = length;
        while (parent_length && path[parent_length - 1] != '/') { parent_length--; }
        if (parent_length > 1) { parent_length--; }
        const SourceCatalogImageEntry *parent = source_image_find(image, path, parent_length);
        return fail(catalog, parent && parent->metadata_captured && S_ISDIR(parent->metadata.st_mode) &&
            parent->children_complete ? MODULE_MISSING : MODULE_IO_ERROR, path);
    }
    if (!entry->metadata_captured) { return fail(catalog, MODULE_IO_ERROR, path); }
    if (!S_ISDIR(entry->metadata.st_mode) && !S_ISREG(entry->metadata.st_mode)) {
        return fail(catalog, MODULE_UNSUPPORTED_PATH, path);
    }
    *output = entry->metadata; return MODULE_OK;
}
ModuleStatus source_image_root(void *context, ModuleCatalog *catalog, const char *path, struct stat *output) {
    const SourceCatalogImage *image = context;
    if (!image || !image->valid || !path || !output) { return fail(catalog, MODULE_INVALID_PATH, path); }
    const char *root = NULL;
    for (size_t i = 0; i < source_image_bound_count(image); i++) {
        const char *candidate = source_image_bound_path(image, i);
        if (source_image_below(path, candidate) && (!root || strlen(candidate) > strlen(root))) { root = candidate; }
    }
    if (!root) { return fail(catalog, MODULE_INVALID_PATH, path); }
    struct stat target;
    ModuleStatus status = lookup(image, catalog, path, strlen(path), &target);
    if (status != MODULE_OK) { return status; }
    if (!S_ISDIR(target.st_mode)) { return fail(catalog, MODULE_UNSUPPORTED_PATH, path); }
    size_t length = strlen(path), boundary = strlen(root);
    while (length > boundary) {
        while (length && path[length - 1] != '/') { length--; }
        if (length > 1) { length--; }
        struct stat parent;
        status = lookup(image, catalog, path, length, &parent);
        if (status != MODULE_OK) { return status; }
        if (!S_ISDIR(parent.st_mode)) { return fail(catalog, MODULE_UNSUPPORTED_PATH, path); }
    }
    *output = target; return MODULE_OK;
}
ModuleStatus source_image_child(void *context, ModuleCatalog *catalog, const char *parent,
    const char *name, const char *full, struct stat *output) {
    if (!parent || !name || !*name || strchr(name, '/') || !full || !output) {
        return fail(catalog, MODULE_INVALID_PATH, full);
    }
    size_t length = strlen(parent);
    if (strncmp(parent, full, length) || full[length] != '/' || strcmp(full + length + 1, name)) {
        return fail(catalog, MODULE_INVALID_PATH, full);
    }
    struct stat directory;
    ModuleStatus status = source_image_root(context, catalog, parent, &directory);
    return status == MODULE_OK ? lookup(context, catalog, full, strlen(full), output) : status;
}
