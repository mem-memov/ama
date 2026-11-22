/** @file
 * @brief Assemble exact simultaneous image changes and admit their final topology.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires prepared, verified changes before publication; no host writes occur.
 */
#include "projection.h"
#include "internal.h"
#include <stdint.h>
#include <string.h>
/** A complete observed directory or known regular ancestor proves missing descendants. */
static bool absent(const SourceCatalogImage *image, const char *path) {
    size_t length = strlen(path);
    while (length) {
        const SourceCatalogImageEntry *row = source_image_find(image, path, length);
        if (row) { return row->metadata_captured && (S_ISREG(row->metadata.st_mode) ||
            (S_ISDIR(row->metadata.st_mode) && row->children_complete)); }
        if (length == 1) { return false; }
        while (length && path[length - 1] != '/') { length--; }
        if (length > 1) { length--; }
    }
    return false;
}
/** A directory needs complete original membership; a known regular parent has no children. */
static bool parent_complete(const SourceCatalogImage *image, const char *path) {
    const char *slash = strrchr(path, '/');
    if (!slash) { return false; }
    size_t length = slash == path ? 1 : (size_t)(slash - path);
    const SourceCatalogImageEntry *parent = source_image_find(image, path, length);
    return parent ? parent->metadata_captured && (S_ISREG(parent->metadata.st_mode) ||
        (S_ISDIR(parent->metadata.st_mode) && parent->children_complete)) : absent(image, path);
}
/** Check original authority before a final row can participate in common discovery. */
static SourceProjectionStatus check(const SourceCatalogImage *image, const SourceImageChange *change) {
    const char *path = change->entry.path;
    if (!path || path[0] != '/' || change->kind < SOURCE_IMAGE_ADD || change->kind > SOURCE_IMAGE_REMOVE) { return SOURCE_PROJECTION_INVALID; }
    for (size_t i = 0; i < image->root_count; i++) {
        if (!strcmp(path, image->roots[i].path)) { return SOURCE_PROJECTION_ROOT; }
    }
    const SourceCatalogImageEntry *old = source_image_find(image, path, strlen(path));
    if (change->kind == SOURCE_IMAGE_ADD) {
        if (old) { return SOURCE_PROJECTION_EXISTS; }
        if (!parent_complete(image, path)) { return SOURCE_PROJECTION_UNOBSERVED; }
    } else {
        if (!old) { return absent(image, path) ? SOURCE_PROJECTION_MISSING : SOURCE_PROJECTION_UNOBSERVED; }
        if (!old->metadata_captured || !parent_complete(image, path) ||
            (S_ISDIR(old->metadata.st_mode) && !old->children_complete)) { return SOURCE_PROJECTION_UNOBSERVED; }
    }
    if (change->kind != SOURCE_IMAGE_REMOVE && (!change->entry.metadata_captured ||
        (S_ISDIR(change->entry.metadata.st_mode) && !change->entry.children_complete))) { return SOURCE_PROJECTION_UNOBSERVED; }
    return SOURCE_PROJECTION_OK;
}
SourceProjectionStatus source_image_project(SourceArena *arena, const SourceCatalogImage *original,
    const SourceImageChange *changes, size_t count, SourceCatalogImage *output, SourceProjectionIssue *issue) {
    if (!issue) { return SOURCE_PROJECTION_INVALID; }
    *issue = (SourceProjectionIssue){.change = SIZE_MAX};
    if (!arena || !original || !original->valid || !output || output == original || (count && !changes)) { return SOURCE_PROJECTION_INVALID; }
    size_t additions = 0;
    for (size_t i = 0; i < count; i++) {
        *issue = (SourceProjectionIssue){i, changes[i].entry.path};
        SourceProjectionStatus status = check(original, &changes[i]);
        if (status != SOURCE_PROJECTION_OK) { return status; }
        for (size_t j = 0; j < i; j++) {
            if (!strcmp(changes[j].entry.path, changes[i].entry.path)) { return SOURCE_PROJECTION_DUPLICATE; }
        }
        additions += changes[i].kind == SOURCE_IMAGE_ADD;
    }
    if (additions > SIZE_MAX - original->count || original->count + additions > SIZE_MAX / sizeof(SourceCatalogImageEntry)) { return SOURCE_PROJECTION_ALLOCATION; }
    size_t capacity = original->count + additions, used = 0;
    SourceCatalogImageEntry *rows = capacity ? source_arena_allocate(arena, capacity * sizeof(*rows)) : NULL;
    if (capacity && !rows) { return SOURCE_PROJECTION_ALLOCATION; }
    for (size_t i = 0; i < original->count; i++) {
        const SourceImageChange *change = NULL;
        for (size_t j = 0; j < count; j++) {
            if (!strcmp(original->entries[i].path, changes[j].entry.path)) { change = &changes[j]; break; }
        }
        if (!change || change->kind != SOURCE_IMAGE_REMOVE) { rows[used++] = change ? change->entry : original->entries[i]; }
    }
    for (size_t i = 0; i < count; i++) { if (changes[i].kind == SOURCE_IMAGE_ADD) { rows[used++] = changes[i].entry; } }
    SourceImageIssue shape;
    if (source_catalog_image_prepare(output, rows, used, original->roots, original->root_count, &shape) != SOURCE_IMAGE_OK) {
        *issue = (SourceProjectionIssue){SIZE_MAX, shape.path}; return SOURCE_PROJECTION_SHAPE;
    }
    *issue = (SourceProjectionIssue){.change = SIZE_MAX}; return SOURCE_PROJECTION_OK;
}
