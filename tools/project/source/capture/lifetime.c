/** @file
 * @brief Retire compiler-input metadata before catalog and raw companion owners.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * preserves destructibility even after incomplete preparation.
 */
#include <string.h>
#include "capture.h"
void source_root_capture_init(SourceRootCapture *capture, SourceAllocator allocator) {
    memset(capture, 0, sizeof(*capture)); module_catalog_init(&capture->catalog, allocator);
    source_arena_init(&capture->arena, allocator); source_manager_init(&capture->files, allocator);
}
void source_root_capture_destroy(SourceRootCapture *capture) {
    source_arena_destroy(&capture->arena); source_manager_destroy(&capture->files);
    module_catalog_destroy(&capture->catalog); memset(capture, 0, sizeof(*capture));
}
