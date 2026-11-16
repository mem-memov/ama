/** @file
 * @brief Compose exact source preparation and independent final binding checks.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * separates a validated plan from later recoverable filesystem publication.
 */
#include "internal.h"
/** A failed or repeated build never authorizes consuming partial staged results. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_staging_build(RelocationStaging *staging, const RelocationStagingRequest *request) {
    if (staging->attempted) { return false; }
    staging->attempted = true;
    const RelocationReferences *references = request == NULL ? NULL : request->references;
    if (references == NULL || !references->ready) {
        return relocation_staging_fail(staging,RELOCATION_INVALID_REFERENCE,"Complete original relocation bindings are required",(SourceSpan){0});
    }
    staging->references = references;
    if (!relocation_staging_texts(staging) || !relocation_staging_files(staging)
        || !relocation_staging_domain(staging)
        || !relocation_staging_graph(staging) || !relocation_staging_settings(staging,request) || !relocation_staging_writes(staging)) { return false; }
    staging->ready = true;
    return true;
}
