/** @file
 * @brief Seal captured original observations into a stable finite catalog image.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * keeps failed preparation unpublishable; [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * consumes exact explicit material through common final discovery.
 */
#include "internal.h"
#include <stdint.h>
SourceObservationStatus source_observation_seal(SourceObservedImage *observed) {
    if (observed->count > SIZE_MAX / sizeof(SourceCatalogImageEntry)) { return observed->status = SOURCE_OBSERVATION_MEMORY; }
    SourceCatalogImageEntry *entries = observed->count
        ? source_arena_allocate(&observed->arena, observed->count * sizeof(*entries)) : NULL;
    if (observed->count && !entries) { return observed->status = SOURCE_OBSERVATION_MEMORY; }
    size_t index = 0;
    for (const SourceObservedEntry *entry = observed->first; entry; entry = entry->next) { entries[index++] = entry->value; }
    SourceImageIssue issue = {0};
    if (source_catalog_image_prepare(&observed->image, entries, observed->count, observed->original->roots,
        observed->original->root_count, observed->image.bounds, &issue) != SOURCE_IMAGE_OK) {
        observed->failed_path = issue.path; return observed->status = SOURCE_OBSERVATION_INVALID;
    }
    observed->sealed = observed->valid = true; return SOURCE_OBSERVATION_OK;
}
