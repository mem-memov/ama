/** @file
 * @brief Retain every failed preview prefix without publishing a partial relocation.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires preparation to leave original paths and bytes intact.
 */
#include "../internal.h"
/** Initialize independent owners; no directory is opened or changed. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
void relocation_preview_init(RelocationPreview *preview, SourceAllocator allocator) {
    *preview = (RelocationPreview){0};
    source_arena_init(&preview->arena,allocator);
    module_catalog_init(&preview->catalog,allocator);
    source_manager_init(&preview->additional_sources,allocator);
}
/** Retire source views before their containing records; cleanup runs no native or Ama code. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
void relocation_preview_destroy(RelocationPreview *preview) {
    source_manager_destroy(&preview->additional_sources);
    module_catalog_destroy(&preview->catalog);
    source_arena_destroy(&preview->arena);
    *preview = (RelocationPreview){0};
}
/** Keep the first cause, so later cleanup or allocation failures cannot disguise preflight provenance. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_fail(RelocationPreview *preview, RelocationStatus status, const char *path, const char *related) {
    if (preview->status == RELOCATION_OK) {
        preview->status = status;
        preview->path = path;
        preview->related_path = related;
    }
    return false;
}
