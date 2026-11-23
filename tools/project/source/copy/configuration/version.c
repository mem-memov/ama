/** @file
 * @brief Plan new-version layout without importing source package configuration.
 * [TOOL-01](../../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * creates only wholly absent versions; [PKG-01](../../../../../spec/project/packages.md#pkg-01)
 * requires package.ini/src/tests, including empty areas.
 */
#include "version.h"
#include "internal.h"
#include "../image/internal.h"
#include <stdint.h>
#include <string.h>
/** Pointer membership validates the caller's actual mode observation. */
static bool owns_mode(const SourceCatalogImage *image, const SourceCatalogImageEntry *mode) {
    for (size_t i = 0; i < image->count; i++) { if (&image->entries[i] == mode) { return mode->metadata_captured && S_ISREG(mode->metadata.st_mode); } }
    return false;
}
/** Classify exact area coverage; an ancestor file is never a substitute for a directory. */
static int area_coverage(const SourceFileMap *files, const char *path) {
    bool covered = false; size_t length = strlen(path);
    for (const SourceFileMapping *row = files->first; row; row = row->next) {
        if (!strcmp(row->destination, path)) { if (!row->original->directory) { return -1; } covered = true; }
        else if (!strncmp(row->destination, path, length) && row->destination[length] == '/') { covered = true; }
    }
    return covered ? 1 : 0;
}
SourceVersionInitializationStatus source_version_initialize(SourceArena *arena, SourceManager *texts,
    const SourceObservedImage *original, const ModuleRoot *final_roots, size_t final_root_count,
    const SourceVersionPlacement *version, const SourceFileMap *files,
    const SourceCatalogImageEntry *file_mode, const SourceImageChange *reserved, size_t reserved_count,
    SourceVersionInitialization *output, SourceVersionInitializationIssue *issue) {
    if (!issue) { return SOURCE_VERSION_INITIALIZATION_INVALID; } *issue = (SourceVersionInitializationIssue){0};
    if (!arena || !texts || !original || !original->valid || !original->sealed || !output
        || !files || !files->ready || files->status != SOURCE_FILE_MAP_OK
        || !source_version_placements_valid(version, 1) || (reserved_count && !reserved)
        || (final_root_count && !final_roots) || !owns_mode(&original->image, file_mode)) { return SOURCE_VERSION_INITIALIZATION_INVALID; }
    issue->path = version->physical;
    SourcePlacedIdentity placed;
    issue->identity_status = source_placement_identity(arena, final_roots, final_root_count,
        version->physical, MODULE_PATH_VERSION, &placed);
    if (issue->identity_status != SOURCE_IDENTITY_OK) {
        return issue->identity_status == SOURCE_IDENTITY_MEMORY ? SOURCE_VERSION_INITIALIZATION_MEMORY : SOURCE_VERSION_INITIALIZATION_PLACEMENT;
    }
    if (strcmp(placed.identity, version->identity)) { return SOURCE_VERSION_INITIALIZATION_PLACEMENT; }
    if (!source_image_path_absent(&original->image, version->physical)) { return SOURCE_VERSION_INITIALIZATION_ABSENCE; }
    const char *paths[3];
    const char *suffixes[] = {"package.ini", "src", "tests"};
    for (size_t i = 0; i < 3; i++) {
        paths[i] = source_arena_join(arena, version->physical, "/", suffixes[i]);
        if (!paths[i]) { return SOURCE_VERSION_INITIALIZATION_MEMORY; }
    }
    for (const SourceFileMapping *row = files->first; row; row = row->next) {
        if (!strcmp(row->destination, paths[0])) { issue->path = paths[0]; return row->original->directory
            ? SOURCE_VERSION_INITIALIZATION_CONFLICT : SOURCE_VERSION_INITIALIZATION_CARRIED; }
    }
    if (reserved_count > SIZE_MAX / sizeof(SourceImageChange) - 3) { return SOURCE_VERSION_INITIALIZATION_LIMIT; }
    SourceImageChange *changes = source_arena_allocate(arena, (reserved_count + 3) * sizeof(*changes));
    if (!changes) { return SOURCE_VERSION_INITIALIZATION_MEMORY; }
    for (size_t i = 0; i < reserved_count; i++) {
        const SourceImageChange *change = &reserved[i];
        if (!change->entry.path || change->entry.path[0] != '/' || change->kind < SOURCE_IMAGE_ADD || change->kind > SOURCE_IMAGE_REMOVE
            || (change->kind != SOURCE_IMAGE_REMOVE && (!change->entry.metadata_captured
                || (S_ISDIR(change->entry.metadata.st_mode) ? !change->entry.children_complete
                    : !S_ISREG(change->entry.metadata.st_mode) || !change->entry.content)))) { return SOURCE_VERSION_INITIALIZATION_INVALID; }
        changes[i] = *change;
    }
    SourceCopyImageBuilder builder = {.arena = arena, .original = &original->image, .changes = changes,
        .count = reserved_count, .capacity = reserved_count + 3};
    const SourceCatalogImageEntry *root = source_configuration_root_mode(&original->image, &final_roots[placed.root]);
    if (!root) { issue->path = final_roots[placed.root].path; return SOURCE_VERSION_INITIALIZATION_PLACEMENT; }
    for (size_t i = 0; i < 3; i++) {
        issue->path = paths[i];
        int coverage = i ? area_coverage(files, paths[i]) : 0;
        if (coverage < 0) { return SOURCE_VERSION_INITIALIZATION_CONFLICT; }
        if (coverage) { continue; }
        if (source_copy_image_change(&builder, paths[i])) { return SOURCE_VERSION_INITIALIZATION_CONFLICT; }
        struct stat metadata = {.st_mode = i ? root->metadata.st_mode : file_mode->metadata.st_mode};
        SourceCopyImageStatus status = source_copy_image_identity(&builder, &metadata);
        if (status != SOURCE_COPY_IMAGE_OK) { return SOURCE_VERSION_INITIALIZATION_LIMIT; }
        changes[builder.count++] = (SourceImageChange){SOURCE_IMAGE_ADD, {.path = paths[i], .metadata = metadata,
            .metadata_captured = true, .children_complete = i != 0}};
    }
    const SourceText *configuration = NULL;
    issue->source_status = source_manager_add(texts, paths[0], (const unsigned char *)"", 0, &configuration);
    if (issue->source_status != SOURCE_OK) {
        return issue->source_status == SOURCE_LIMIT ? SOURCE_VERSION_INITIALIZATION_LIMIT : SOURCE_VERSION_INITIALIZATION_MEMORY;
    }
    changes[reserved_count].entry.content = configuration;
    *output = (SourceVersionInitialization){.original = original, .final_roots = final_roots, .final_root_count = final_root_count,
        .version = *version, .configuration = configuration, .changes = &changes[reserved_count], .count = builder.count - reserved_count};
    *issue = (SourceVersionInitializationIssue){0}; return SOURCE_VERSION_INITIALIZATION_OK;
}
