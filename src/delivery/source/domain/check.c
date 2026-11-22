/** @file
 * @brief Prepare the complete active captured domain for original-graph safety checks.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * includes every active definition, independent of executable reachability;
 * [SYN-04](../../../../spec/language/source-and-names.md#syn-04) retains drafts
 * in the catalog without activating their incomplete bodies.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) retains failed
 * preparation under caller-owned arenas and frontend lifetimes.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * selects explicit captured native authority before ordinary source checking.
 */
#include "internal.h"
bool delivery_source_domain(CompilerFrontend *frontend, SourceArena *arena,
    const DeliverySourceCompanions *companions, const DeliverySourceDefinition **output) {
    if (!frontend || !arena || !output || frontend->attempted || !frontend->project.catalog.built) {
        return false;
    }
    const char *const *required = NULL;
    size_t count = 0;
    if (!delivery_source_domain_required(frontend, arena, companions, &required, &count)) {
        frontend->attempted = true;
        return false;
    }
    if (!compiler_frontend_check_snapshot(frontend, required, count)) { return false; }
    SourceStatus status = delivery_source_export(arena, &frontend->static_inventory, output);
    if (status != SOURCE_OK) {
        frontend->error = (Diagnostic){.code = DIAGNOSTIC_OUT_OF_MEMORY,
            .message = "Cannot retain the complete original dependency export"};
        return false;
    }
    return true;
}
