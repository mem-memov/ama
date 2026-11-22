/** @file
 * @brief Export a checked whole source domain without an executable entry or mutation.
 * [PROJECT-01](../../../spec/implementation/project-tools.md#project-01)
 * keeps project orchestration outside the compiler;
 * [TOOL-02](../../../spec/implementation/tools-and-conformance.md#tool-02)
 * requires every active original definition in explicit selected roots;
 * [COMP-04](../../../spec/implementation/compiler.md#comp-04) preserves exact
 * original facts and rejects incomplete output acknowledgement.
 * [ARCH-03](../../../spec/implementation/compiler.md#arch-03) keeps diagnostic
 * and source owners alive through reporting and destroys borrowed exports first.
 */
#include <stdlib.h>
#include "commands.h"
#include "delivery/source/domain/domain.h"
#include "delivery/source/report/report.h"
#include "diagnostics/render/render.h"
#include "source-domain/input.h"
int driver_source_domain(const DriverOptions *options, FILE *output, FILE *errors) {
    CompilerFrontend frontend;
    SourceArena exports;
    SourceManager requests;
    DeliverySourceSnapshot snapshot;
    SourceAllocator allocator = source_system_allocator();
    compiler_frontend_init(&frontend, allocator);
    source_arena_init(&exports, allocator);
    source_manager_init(&requests, allocator);
    delivery_source_snapshot_init(&snapshot, allocator);
    DiagnosticRelated related = {0};
    const DeliverySourceDefinition *definitions = NULL;
    bool checked = false;
    if (options->source_snapshot) {
        checked = driver_source_domain_input(&frontend, &exports, &requests, &snapshot, options->source_snapshot, &definitions);
    } else {
        ModuleStatus status = module_catalog_build(&frontend.project.catalog, options->roots, options->root_count);
        if (status == MODULE_OK) { checked = delivery_source_domain(&frontend, &exports, NULL, &definitions); }
        else { (void)diagnostic_from_module(&frontend.project.catalog.issue, &related, &frontend.error); }
    }
    int result = EXIT_FAILURE;
    if (checked && driver_warnings(&frontend.project, errors)) {
        if (delivery_source_report(output, &frontend.project.catalog, definitions)) { result = EXIT_SUCCESS; }
        else {
            Diagnostic error = {.code = DIAGNOSTIC_IO_FAILURE, .message = "Cannot write the complete checked source-domain response"};
            (void)diagnostic_render(errors, &error);
        }
    } else if (!checked) { (void)diagnostic_render(errors, &frontend.error); }
    source_arena_destroy(&exports);
    compiler_frontend_destroy(&frontend);
    delivery_source_snapshot_destroy(&snapshot);
    source_manager_destroy(&requests);
    return result;
}
