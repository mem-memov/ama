/** @file
 * @brief Prepare the ordinary complete relocation graph and explicit settings before any source publication.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * preserves resolved identities; [TOOL-03](../../../../spec/implementation/tools-and-conformance.md#tool-03)
 * stages only explicit supported configuration/output changes.
 */
#include "internal.h"
/** Allocation authority outlives all independent owners and their diagnostics. */
void driver_relocation_plan_init(DriverRelocationPlan *plan, SourceAllocator allocator) {
    *plan = (DriverRelocationPlan){0};
    relocation_preview_init(&plan->preview,allocator);
    relocation_references_init(&plan->references,allocator);
    relocation_staging_init(&plan->staging,allocator);
    relocation_report_init(&plan->report,allocator);
}
/** No owner destruction attempts publication or discards pending filesystem recovery state. */
void driver_relocation_plan_destroy(DriverRelocationPlan *plan) {
    relocation_report_destroy(&plan->report);
    relocation_staging_destroy(&plan->staging);
    relocation_references_destroy(&plan->references);
    relocation_preview_destroy(&plan->preview);
    *plan = (DriverRelocationPlan){0};
}
/** Failed stage views are used only for diagnostics; a complete report requires every proof stage to pass. */
bool driver_relocation_plan_build(DriverRelocationPlan *plan, const DriverOptions *options) {
    RelocationRequest request = {.roots=options->roots,.root_count=options->root_count,.source=options->relocation_source,
        .destination=options->destination,.destination_root=options->destination_root.path == NULL ? NULL : &options->destination_root};
    if (!relocation_preview_build(&plan->preview,&request)) {
        driver_relocation_preview_error(plan);
        return false;
    }
    if (!relocation_references_build(&plan->references,&plan->preview)) {
        plan->error = &plan->references.error; plan->status = plan->references.status; return false;
    }
    RelocationStagingRequest staging = {.references=&plan->references,.entry=options->entry,
        .library_configurations=options->configurations,.library_configuration_count=options->configuration_count,
        .generated_outputs=options->invalidations,.generated_output_count=options->invalidation_count};
    if (!relocation_staging_build(&plan->staging,&staging)) {
        plan->error = &plan->staging.error; plan->status = plan->staging.status; return false;
    }
    if (!relocation_report_build(&plan->report,&plan->staging)) {
        plan->failure = (Diagnostic){.code=DIAGNOSTIC_OUT_OF_MEMORY,.message="The relocation response scope could not be owned"};
        plan->error = &plan->failure; plan->status = plan->report.status; return false;
    }
    return true;
}
