/** @file
 * @brief Compose explicit request-file I/O, strict capture decoding and ordinary frontend admission.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * checks captured future paths without rereading them;
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves
 * original request/source diagnostics;
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) retains borrowed
 * input owners through complete reporting and reverse teardown.
 */
#include "input.h"
bool driver_source_domain_input(CompilerFrontend *frontend, SourceArena *exports, SourceManager *requests,
    DeliverySourceSnapshot *snapshot, const char *path, const DeliverySourceDefinition **output) {
    const SourceText *source = NULL;
    SourceStatus loaded = source_manager_load(requests, path, &source);
    if (loaded != SOURCE_OK) {
        frontend->error = (Diagnostic){
            .code = loaded == SOURCE_OUT_OF_MEMORY || loaded == SOURCE_LIMIT ? DIAGNOSTIC_OUT_OF_MEMORY : DIAGNOSTIC_IO_FAILURE,
            .message = "Cannot read the explicit captured source input", .primary = {.path = path}};
        return false;
    }
    SourceJsonStatus status = delivery_source_snapshot_read(snapshot, source);
    if (status != SOURCE_JSON_OK) {
        frontend->error = (Diagnostic){
            .code = status == SOURCE_JSON_OUT_OF_MEMORY ? DIAGNOSTIC_OUT_OF_MEMORY : DIAGNOSTIC_INVALID_PATH,
            .message = snapshot->issue.message, .primary = {.span = snapshot->issue.origin}};
        return false;
    }
    return delivery_source_domain_snapshot(frontend, exports, snapshot, output);
}
