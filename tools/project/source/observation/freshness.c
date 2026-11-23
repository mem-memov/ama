/** @file
 * @brief Revalidate exact observation domains, physical classes and captured bytes.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * treats added directory names as stale even when the active module graph is unchanged.
 */
#include "freshness.h"
#include <string.h>
/** Root order and exclusion spelling are part of the supplied analysis boundary. */
static bool same_roots(const SourceCatalogImage *original, const SourceCatalogImage *fresh) {
    if (original->root_count != fresh->root_count) { return false; }
    for (size_t i = 0; i < original->root_count; i++) {
        const ModuleRoot *a = &original->roots[i], *b = &fresh->roots[i];
        if (a->kind != b->kind || strcmp(a->path, b->path) || a->excluded_count != b->excluded_count) { return false; }
        for (size_t j = 0; j < a->excluded_count; j++) { if (strcmp(a->excluded[j], b->excluded[j])) { return false; } }
    }
    return true;
}
/** Exact keys allow reordered independent observations without alias guessing. */
static const SourceCatalogImageEntry *find(const SourceCatalogImage *image, const char *path) {
    for (size_t i = 0; i < image->count; i++) { if (!strcmp(image->entries[i].path, path)) { return &image->entries[i]; } }
    return NULL;
}
SourceFreshnessStatus source_observation_freshness(const SourceCatalogImage *original,
    const SourceCatalogImage *fresh, SourceFreshnessIssue *issue) {
    if (!issue) { return SOURCE_FRESH_INVALID; } *issue = (SourceFreshnessIssue){0};
    if (!original || !original->valid || !fresh || !fresh->valid) { return SOURCE_FRESH_INVALID; }
    if (!same_roots(original, fresh) || !source_image_same_bounds(original, fresh)) { return SOURCE_FRESH_ROOTS; }
    for (size_t i = 0; i < original->count; i++) {
        const SourceCatalogImageEntry *a = &original->entries[i], *b = find(fresh, a->path);
        *issue = (SourceFreshnessIssue){a->path, a, b};
        if (!b) { return SOURCE_FRESH_MEMBERSHIP; }
        if (a->metadata_captured != b->metadata_captured || a->children_complete != b->children_complete
            || (a->content != NULL) != (b->content != NULL)) { return SOURCE_FRESH_OBSERVATION; }
        if (a->metadata_captured && (a->metadata.st_dev != b->metadata.st_dev
            || a->metadata.st_ino != b->metadata.st_ino || a->metadata.st_mode != b->metadata.st_mode)) { return SOURCE_FRESH_IDENTITY; }
        if (a->content && (source_length(a->content) != source_length(b->content)
            || memcmp(source_bytes(a->content), source_bytes(b->content), source_length(a->content)))) { return SOURCE_FRESH_CONTENT; }
    }
    if (original->count != fresh->count) {
        for (size_t i = 0; i < fresh->count; i++) {
            const SourceCatalogImageEntry *entry = &fresh->entries[i];
            if (!find(original, entry->path)) { *issue = (SourceFreshnessIssue){entry->path, NULL, entry}; return SOURCE_FRESH_MEMBERSHIP; }
        }
    }
    *issue = (SourceFreshnessIssue){0}; return SOURCE_FRESH_OK;
}
