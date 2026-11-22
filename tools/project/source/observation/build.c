/** @file
 * @brief Admit original common-catalog observations before publishing an immutable image.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * uses the complete original source scope; [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * rejects changed main/configuration/companion observations without source mutation.
 */
#include "internal.h"
/** Preserve a more precise callback failure instead of replacing it with a
 * generic catalog result under [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04).
 */
static SourceObservationStatus catalog_failure(SourceObservedImage *observed) {
    if (observed->status == SOURCE_OBSERVATION_OK) {
        observed->failed_path = observed->catalog.issue.path;
        observed->status = observed->catalog.issue.status == MODULE_OUT_OF_MEMORY
            ? SOURCE_OBSERVATION_MEMORY : SOURCE_OBSERVATION_CATALOG;
    }
    return observed->status;
}
SourceObservationStatus source_observed_image_build(SourceObservedImage *observed, const SourceRootCapture *original,
    const SourceObservationRequest *requests, size_t count) {
    if (!observed || observed->attempted) { return SOURCE_OBSERVATION_INVALID; }
    observed->attempted = true;
    if (!original || !original->valid || (count && !requests)) { return observed->status = SOURCE_OBSERVATION_INVALID; }
    observed->original = original;
    if (module_catalog_build_view(&observed->catalog, original->roots, original->root_count, &observed->reader) != MODULE_OK) {
        return catalog_failure(observed);
    }
    for (const ModuleSource *source = observed->catalog.first; source; source = source->next) {
        if (module_catalog_load_configuration(&observed->catalog, source) != MODULE_OK) { return catalog_failure(observed); }
    }
    if (source_observation_compare(observed) != SOURCE_OBSERVATION_OK) { return observed->status; }
    if (source_observation_units(observed) != SOURCE_OBSERVATION_OK) { return observed->status; }
    if (source_observation_requests(observed, requests, count) != SOURCE_OBSERVATION_OK) { return observed->status; }
    return source_observation_seal(observed);
}
