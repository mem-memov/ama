/** @file
 * @brief Separate removed-material demands and consistency checks from orchestration.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * rejects changed original opaque inputs before preparing a final image.
 */
#ifndef AMA_SOURCE_REMOVAL_IMAGE_INTERNAL_H
#define AMA_SOURCE_REMOVAL_IMAGE_INTERNAL_H
#include "image.h"
/** Own request rows in preparation arena; borrow exact original selected keys. */
SourceRemovalImageStatus source_removal_image_requests(SourceRemovalImage *, const SourceObservationRequest **);
/** Compare all removed bytes/shapes, then create one explicit removal per key. */
SourceRemovalImageStatus source_removal_image_changes(SourceRemovalImage *, const SourceImageChange **);
#endif
