/** @file
 * @brief Separate complete image-unit capture from shared snapshot table construction.
 * [PKG-07](../../../../../spec/project/packages.md#pkg-07) retains every dot
 * companion; [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * forbids incomplete or role-guessed compiler input captures.
 */
#ifndef AMA_PROJECT_SOURCE_PREPARED_CAPTURE_INTERNAL_H
#define AMA_PROJECT_SOURCE_PREPARED_CAPTURE_INTERNAL_H
#include "../image.h"
#include "../internal.h"
/** Capture one catalog-selected main and all regular dot companions under owned
 * final keys. Publish the unit only after all bytes/records succeed. No I/O.
 */
SourceImageCaptureStatus source_image_capture_unit(SourceImageCapture *, const ModuleSource *, SourceCapturedUnit **);
#endif
