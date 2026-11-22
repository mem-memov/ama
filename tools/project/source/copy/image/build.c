/** @file
 * @brief Bound and orchestrate one simultaneous material/scaffolding projection.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires validated immutable preparation before any source publication.
 */
#include "internal.h"
#include <stdint.h>
#include <string.h>
/** Count a conservative finite ancestor-slot bound without allocation or wrap. */
static bool reserve_path(size_t *capacity, const char *path) {
    if (!path || path[0] != '/') { return false; }
    for (const char *cursor = path; *cursor; cursor++) {
        if (*cursor == '/') { if (*capacity == SIZE_MAX) { return false; } (*capacity)++; }
    }
    return true;
}
SourceCopyImageStatus source_copy_image_prepare(SourceArena *arena, const SourceCatalogImage *original,
    const SourceFileMap *map, const SourceGraphPreparedText *prepared, const SourceImageChange *extra, size_t extra_count,
    const SourceCatalogImage **output, SourceCopyImageIssue *issue) {
    if (!issue) { return SOURCE_COPY_IMAGE_INVALID; } *issue = (SourceCopyImageIssue){0};
    if (!arena || !original || !original->valid || !output || (extra_count && !extra)) { return SOURCE_COPY_IMAGE_INVALID; }
    SourceCopyMaterial material = {0};
    issue->material_status = source_copy_material_prepare(arena, map, prepared, &material, &issue->material);
    if (issue->material_status != SOURCE_MATERIAL_OK) { return SOURCE_COPY_IMAGE_MATERIAL; }
    if (extra_count > SIZE_MAX - material.count) { return SOURCE_COPY_IMAGE_LIMIT; }
    size_t capacity = extra_count + material.count;
    for (size_t i = 0; i < extra_count; i++) {
        issue->path = extra[i].entry.path;
        if (!extra[i].entry.path || extra[i].entry.path[0] != '/' || extra[i].kind < SOURCE_IMAGE_ADD || extra[i].kind > SOURCE_IMAGE_REMOVE) { return SOURCE_COPY_IMAGE_INVALID; }
        for (size_t j = 0; j < i; j++) { if (!strcmp(extra[i].entry.path, extra[j].entry.path)) { return SOURCE_COPY_IMAGE_COLLISION; } }
        if (!reserve_path(&capacity, extra[i].entry.path)) { return SOURCE_COPY_IMAGE_LIMIT; }
    }
    for (size_t i = 0; i < material.count; i++) {
        if (!reserve_path(&capacity, material.entries[i].mapping->destination)) { return SOURCE_COPY_IMAGE_LIMIT; }
    }
    if (capacity > SIZE_MAX / sizeof(SourceImageChange)) { return SOURCE_COPY_IMAGE_LIMIT; }
    SourceImageChange *changes = capacity ? source_arena_allocate(arena, capacity * sizeof(*changes)) : NULL;
    if (capacity && !changes) { return SOURCE_COPY_IMAGE_MEMORY; }
    for (size_t i = 0; i < extra_count; i++) { changes[i] = extra[i]; }
    SourceCopyImageBuilder builder = {arena, original, changes, extra_count, capacity, issue};
    SourceCopyImageStatus status = source_copy_image_members(&builder, &material);
    if (status != SOURCE_COPY_IMAGE_OK) { return status; }
    issue->mapping = NULL;
    status = source_copy_image_parents(&builder);
    if (status != SOURCE_COPY_IMAGE_OK) { return status; }
    SourceCatalogImage *image = source_arena_allocate(arena, sizeof(*image));
    if (!image) { return SOURCE_COPY_IMAGE_MEMORY; }
    issue->projection_status = source_image_project(arena, original, changes, builder.count, image, &issue->projection);
    if (issue->projection_status != SOURCE_PROJECTION_OK) { return SOURCE_COPY_IMAGE_PROJECTION; }
    *output = image; *issue = (SourceCopyImageIssue){0}; return SOURCE_COPY_IMAGE_OK;
}
