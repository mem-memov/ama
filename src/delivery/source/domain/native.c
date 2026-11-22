/** @file
 * @brief Bind exact captured companions after native classification and before source checking.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * prevents staged paths from triggering physical companion fallback;
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) retains
 * prepared/failing catalog metadata and original diagnostics through teardown.
 */
#include <string.h>
#include "internal.h"
/** A missing/duplicate/already-bound authority is an input error, never an
 * invitation to choose a row or probe a future path under
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04).
 */
static bool invalid(CompilerFrontend *frontend, const ModuleSource *source, const char *message) {
    frontend->error = (Diagnostic){.code = DIAGNOSTIC_CHANGED_INPUT, .message = message,
        .primary = {.span = {source->text, 0, 0}}};
    return false;
}
bool delivery_source_domain_native(CompilerFrontend *frontend, const ModuleSource *source,
    const DeliverySourceCompanions *companions) {
    if (source->native_loaded) { return invalid(frontend, source, "Captured native authority was already installed"); }
    const DeliverySourceCompanions *selected = NULL;
    for (const DeliverySourceCompanions *item = companions; item; item = item->next) {
        if (!item->identity) { return invalid(frontend, source, "Captured companion row has no source identity"); }
        if (strcmp(item->identity, source->logical_path)) { continue; }
        if (selected) { return invalid(frontend, source, "Native source has duplicate captured companion rows"); }
        selected = item;
    }
    if (!selected) { return invalid(frontend, source, "Native source has no authoritative captured companions"); }
    ModuleStatus status = module_catalog_snapshot_native(&frontend->project.catalog, source,
        selected->body, selected->header, selected->abi);
    if (status == MODULE_OK) { return true; }
    if (!diagnostic_from_module(&frontend->project.catalog.issue, &frontend->project.catalog_related, &frontend->error)) {
        return invalid(frontend, source, "Captured native companion authority cannot be installed");
    }
    return false;
}
