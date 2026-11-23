/** @file
 * @brief Preserve complete ignored original units without treating retirement as deletion permission.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * preserves original main/companion material, even after a different copy supplies
 * the same nominal Type. [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires complete observed membership rather than assuming missing companions.
 */
#include <string.h>
#include "internal.h"
/** Exact immutable image keys require neither path normalization nor host probes. */
static const SourceCatalogImageEntry *entry(const SourceCatalogImage *image, const char *path, size_t length) {
    for (size_t i = 0; i < image->count; i++) {
        const char *key = image->entries[i].path;
        if (strlen(key) == length && !memcmp(key, path, length)) { return &image->entries[i]; }
    }
    return NULL;
}
/** Unit capture includes exactly its main and immediate basename-dot-nonempty
 * companions; a plus subtree remains a distinct structural-material obligation. */
static bool membership(const SourceCapturedUnit *old, const SourceCatalogImage *image) {
    const char *main = source_path(old->source->text), *slash = strrchr(main, '/');
    if (!slash) { return false; }
    size_t parent_length = slash == main ? 1 : (size_t)(slash - main), length = strlen(main);
    const SourceCatalogImageEntry *parent = entry(image, main, parent_length);
    if (!parent || !parent->metadata_captured || !S_ISDIR(parent->metadata.st_mode) || !parent->children_complete) { return false; }
    for (size_t i = 0; i < image->count; i++) {
        const char *path = image->entries[i].path;
        if (strncmp(path, main, length) || path[length] != '.' || !path[length + 1] || strchr(path + length + 1, '/')) { continue; }
        const AcquisitionUnitEntry *file = old->files;
        while (file && strcmp(file->file.path, path)) { file = file->next; }
        if (!file) { return false; }
    }
    return true;
}
bool source_copy_proof_preserved_unit(const SourceCapturedUnit *old, const SourceCatalogImage *image) {
    if (!membership(old, image)) { return false; }
    for (const AcquisitionUnitEntry *file = old->files; file; file = file->next) {
        const SourceCatalogImageEntry *row = entry(image, file->file.path, strlen(file->file.path));
        if (!row || !row->metadata_captured || (file->directory ? !S_ISDIR(row->metadata.st_mode)
            : !S_ISREG(row->metadata.st_mode) || !row->content || source_length(file->file.content) != source_length(row->content)
                || memcmp(source_bytes(file->file.content), source_bytes(row->content), source_length(row->content)))) { return false; }
    }
    return true;
}
