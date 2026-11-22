/** @file
 * @brief Enumerate only declared complete immediate image memberships in bytewise order.
 * [COMP-05](../../../../spec/implementation/compiler.md#comp-05) requires stable
 * ordering; [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * forbids inventing absent children from an incomplete observation.
 */
#include "internal.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
/** Exact immediate parent relation; opaque nested data is not a module-name guess. */
static const char *child_name(const char *path, const char *parent) {
    const char *slash = strrchr(path, '/');
    size_t length = slash == path ? 1 : (size_t)(slash - path);
    return slash[1] && strlen(parent) == length && !memcmp(path, parent, length) ? slash + 1 : NULL;
}
/** Sort borrowed immutable entry names, never locale-normalize their bytes. */
static int compare_names(const void *left, const void *right) {
    return strcmp(*(const char *const *)left, *(const char *const *)right);
}
ModuleStatus source_image_entries(void *context, ModuleCatalog *catalog, const char *path,
    const char ***output, size_t *count) {
    const SourceCatalogImage *image = context; struct stat metadata;
    ModuleStatus status = source_image_root(context, catalog, path, &metadata);
    if (status != MODULE_OK) { return status; }
    const SourceCatalogImageEntry *directory = source_image_find(image, path, strlen(path));
    if (!directory->children_complete) {
        catalog->issue = (ModuleIssue){.status = MODULE_IO_ERROR, .path = path}; return MODULE_IO_ERROR;
    }
    size_t children = 0;
    for (size_t i = 0; i < image->count; i++) { children += child_name(image->entries[i].path, path) != NULL; }
    if (children > SIZE_MAX / sizeof(const char *)) {
        catalog->issue = (ModuleIssue){.status = MODULE_OUT_OF_MEMORY, .path = path}; return MODULE_OUT_OF_MEMORY;
    }
    const char **names = children ? source_arena_allocate(&catalog->arena, children * sizeof(*names)) : NULL;
    if (children && !names) {
        catalog->issue = (ModuleIssue){.status = MODULE_OUT_OF_MEMORY, .path = path}; return MODULE_OUT_OF_MEMORY;
    }
    size_t used = 0;
    for (size_t i = 0; i < image->count; i++) {
        const char *name = child_name(image->entries[i].path, path);
        if (name) { names[used++] = name; }
    }
    if (children) { qsort(names, children, sizeof(*names), compare_names); }
    *output = names; *count = children; return MODULE_OK;
}
