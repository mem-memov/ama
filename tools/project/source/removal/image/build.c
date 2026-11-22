/** @file
 * @brief Join original removal safety, immutable material projection and final common capture.
 * [TOOL-03](../../../../../spec/implementation/tools-and-conformance.md#tool-03)
 * runs original cuts before treating final availability as evidence;
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * reserves semantic/freshness/publication acceptance for the outer operation.
 */
#include "internal.h"
SourceRemovalImageStatus source_removal_image_build(SourceRemovalImage *image, SourceRemovalSelection *selection,
    const SourceRemovalInputUse *inputs, const SourceClientRoot *roots) {
    if (!image || image->attempted) { return SOURCE_REMOVAL_IMAGE_INVALID; }
    image->attempted = true;
    if (!selection || !selection->valid) { return image->status = SOURCE_REMOVAL_IMAGE_INVALID; }
    image->selection = selection;
    image->review_status = source_removal_selection_review(selection, inputs, roots, &image->review);
    if (image->review_status != SOURCE_REMOVAL_OK) { return image->status = SOURCE_REMOVAL_IMAGE_REVIEW; }
    const SourceObservationRequest *requests = NULL; const SourceImageChange *changes = NULL;
    image->status = source_removal_image_requests(image, &requests);
    if (image->status != SOURCE_REMOVAL_IMAGE_OK) { return image->status; }
    if (source_observed_image_build(&image->observed, selection->tree->capture,
        requests, selection->path_count) != SOURCE_OBSERVATION_OK) {
        image->failed_path = image->observed.failed_path; return image->status = SOURCE_REMOVAL_IMAGE_OBSERVATION;
    }
    image->status = source_removal_image_changes(image, &changes);
    if (image->status != SOURCE_REMOVAL_IMAGE_OK) { return image->status; }
    image->projection_status = source_image_project(&image->arena, &image->observed.image, changes,
        selection->path_count, &image->image, &image->projection_issue);
    if (image->projection_status != SOURCE_PROJECTION_OK) {
        image->failed_path = image->projection_issue.path; return image->status = SOURCE_REMOVAL_IMAGE_PROJECTION;
    }
    if (source_image_capture_build(&image->capture, &image->image) != SOURCE_IMAGE_CAPTURE_OK) {
        image->failed_path = image->capture.failed_path; return image->status = SOURCE_REMOVAL_IMAGE_CAPTURE;
    }
    image->valid = true; return SOURCE_REMOVAL_IMAGE_OK;
}
