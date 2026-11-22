/** @file
 * @brief Join common final discovery, complete image units and one shared snapshot protocol.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * makes actual final selection/topology authoritative;
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires independent source typing and reference/freshness validation afterward.
 */
#include "internal.h"
SourceImageCaptureStatus source_image_capture_build(SourceImageCapture *capture, const SourceCatalogImage *image) {
    if (!capture || capture->attempted) { return SOURCE_IMAGE_CAPTURE_INVALID; }
    capture->attempted = true;
    if (!image || !image->valid) { return capture->status = SOURCE_IMAGE_CAPTURE_INVALID; }
    capture->image = image;
    if (module_catalog_build_view(&capture->catalog, image->roots, image->root_count, &image->reader) != MODULE_OK) {
        capture->failed_path = capture->catalog.issue.path;
        return capture->status = capture->catalog.issue.status == MODULE_OUT_OF_MEMORY
            ? SOURCE_IMAGE_CAPTURE_MEMORY : SOURCE_IMAGE_CAPTURE_CATALOG;
    }
    SourceCapturedUnit *first = NULL, **tail = &first;
    for (const ModuleSource *source = capture->catalog.first; source; source = source->next) {
        if (module_catalog_load_configuration(&capture->catalog, source) != MODULE_OK) {
            capture->failed_path = capture->catalog.issue.path;
            return capture->status = capture->catalog.issue.status == MODULE_OUT_OF_MEMORY
                ? SOURCE_IMAGE_CAPTURE_MEMORY : SOURCE_IMAGE_CAPTURE_CATALOG;
        }
        SourceCapturedUnit *unit = NULL;
        capture->status = source_image_capture_unit(capture, source, &unit);
        if (capture->status != SOURCE_IMAGE_CAPTURE_OK) { return capture->status; }
        *tail = unit; tail = &unit->next;
    }
    capture->units = first;
    capture->table_status = source_capture_tables(&capture->arena, &capture->catalog, first, &capture->view);
    if (capture->table_status != SOURCE_CAPTURE_OK) {
        return capture->status = capture->table_status == SOURCE_CAPTURE_MEMORY ? SOURCE_IMAGE_CAPTURE_MEMORY :
            capture->table_status == SOURCE_CAPTURE_LIMIT ? SOURCE_IMAGE_CAPTURE_LIMIT : SOURCE_IMAGE_CAPTURE_INVALID;
    }
    capture->valid = true; capture->failed_path = NULL; return SOURCE_IMAGE_CAPTURE_OK;
}
