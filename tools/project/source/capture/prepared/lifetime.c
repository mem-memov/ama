/** @file
 * @brief Own prepared compiler-input metadata independently of physical original captures.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) keeps failed
 * prefixes destructible; [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * retains original/projected input authority in distinct owners.
 */
#include "../image.h"
#include <string.h>
void source_image_capture_init(SourceImageCapture *capture, SourceAllocator allocator) {
    memset(capture, 0, sizeof(*capture)); module_catalog_init(&capture->catalog, allocator);
    source_arena_init(&capture->arena, allocator); source_manager_init(&capture->files, allocator);
}
void source_image_capture_destroy(SourceImageCapture *capture) {
    source_arena_destroy(&capture->arena); source_manager_destroy(&capture->files);
    module_catalog_destroy(&capture->catalog); memset(capture, 0, sizeof(*capture));
}
