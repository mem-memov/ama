/** @file
 * @brief Own removal-image preparation in reverse borrower lifetime order.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) keeps failed
 * prefixes destructible; [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * separates scratch preparation from physical publication.
 */
#include "image.h"
#include <string.h>
void source_removal_image_init(SourceRemovalImage *image, SourceAllocator allocator) {
    memset(image, 0, sizeof(*image)); source_arena_init(&image->arena, allocator);
    source_observed_image_init(&image->observed, allocator); source_image_capture_init(&image->capture, allocator);
}
void source_removal_image_destroy(SourceRemovalImage *image) {
    source_image_capture_destroy(&image->capture); source_observed_image_destroy(&image->observed);
    source_arena_destroy(&image->arena); image->image = (SourceCatalogImage){0}; image->valid = false;
}
