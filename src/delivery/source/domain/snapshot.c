/** @file
 * @brief Check explicit captured analysis identities through the ordinary snapshot frontend.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * retains original/newly covered analysis identities;
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * forbids physical fallback while checking prepared staged bytes;
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) keeps partial
 * catalog preparation destructible and successful outputs frontend-owned.
 */
#include "internal.h"
/** A failed preparation consumes this frontend even before normal project
 * loading starts under [ARCH-03](../../../../spec/implementation/compiler.md#arch-03).
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires original capture diagnostics rather than a new filesystem attempt.
 */
static bool failed(CompilerFrontend *frontend) {
    frontend->attempted = true;
    if (!diagnostic_from_module(&frontend->project.catalog.issue,
        &frontend->project.catalog_related, &frontend->error)) {
        frontend->error = (Diagnostic){.code = DIAGNOSTIC_CHANGED_INPUT,
            .message = "Captured source registry could not be prepared"};
    }
    return false;
}
bool delivery_source_snapshot_catalog(CompilerFrontend *frontend, const DeliverySourceSnapshot *snapshot) {
    if (!frontend || !snapshot || !snapshot->valid || frontend->attempted
        || frontend->project.catalog.attempted || frontend->project.catalog.versions
        || (snapshot->count && (!snapshot->entries || !snapshot->companions))) { return false; }
    for (const DeliverySnapshotVersion *version = snapshot->versions; version; version = version->next) {
        ModuleStatus status = module_catalog_capture_version(&frontend->project.catalog,
            version->identity, version->directory, version->configuration);
        if (status != MODULE_OK) { return failed(frontend); }
    }
    if (module_catalog_snapshot(&frontend->project.catalog, snapshot->entries, snapshot->count) != MODULE_OK) {
        return failed(frontend);
    }
    return true;
}
bool delivery_source_domain_snapshot(CompilerFrontend *frontend, SourceArena *arena,
    const DeliverySourceSnapshot *snapshot, const DeliverySourceDefinition **output) {
    return arena && output && delivery_source_snapshot_catalog(frontend, snapshot)
        && delivery_source_domain(frontend, arena, snapshot->companions, output);
}
