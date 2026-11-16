/** @file
 * @brief Orchestrate a read-only relocation preflight before any publication API exists.
 * [TOOL-01](../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * requires exact destination and complete scope reporting;
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * leaves original files intact when any preparation phase fails.
 */
#include "../internal.h"
/** Build once; every prefix remains owned for diagnostics, and ready is published only after all phases. Component contract: [TOOL-01](../../../../spec/implementation/tools-and-conformance.md#tool-01), [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_preview_build(RelocationPreview *preview, const RelocationRequest *request) {
    if (preview->attempted) { return false; }
    preview->attempted = true;
    if (request == NULL) { return relocation_fail(preview,RELOCATION_INVALID_PATH,NULL,NULL); }
    if (!relocation_endpoints(preview,request) || !relocation_roots(preview,request)
        || !relocation_inventory(preview) || !relocation_discover(preview)) { return false; }
    preview->ready = true;
    return true;
}
