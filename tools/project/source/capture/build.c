/** @file
 * @brief Prepare every selected catalog unit before publishing a compiler-input view.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * forbids entry-only analysis;
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * refuses partial preparation without source mutation or recapture fallback.
 */
#include "internal.h"
SourceCaptureStatus source_root_capture_build(SourceRootCapture *capture, const ModuleRoot *roots, size_t count) {
    if (!capture || capture->attempted || (count && !roots)) { return SOURCE_CAPTURE_INVALID; }
    capture->attempted = true;
    capture->status = source_capture_roots(capture, roots, count);
    if (capture->status != SOURCE_CAPTURE_OK) { return capture->status; }
    if (module_catalog_build(&capture->catalog, capture->roots, capture->root_count) != MODULE_OK) {
        capture->failed_path = capture->catalog.issue.path;
        return capture->status = SOURCE_CAPTURE_CATALOG;
    }
    SourceCapturedUnit *first = NULL, **tail = &first;
    for (const ModuleSource *source = capture->catalog.first; source; source = source->next) {
        if (module_catalog_load_configuration(&capture->catalog, source) != MODULE_OK) {
            capture->failed_path = capture->catalog.issue.path;
            return capture->status = SOURCE_CAPTURE_CATALOG;
        }
        SourceCapturedUnit *unit = NULL;
        capture->status = source_capture_unit(capture, source, &unit);
        if (capture->status != SOURCE_CAPTURE_OK) { return capture->status; }
        *tail = unit; tail = &unit->next;
    }
    capture->units = first;
    capture->failed_path = NULL;
    capture->status = source_capture_tables(&capture->arena, &capture->catalog, capture->units, &capture->view);
    if (capture->status == SOURCE_CAPTURE_OK) { capture->valid = true; capture->failed_path = NULL; }
    return capture->status;
}
