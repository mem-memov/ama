/** @file
 * @brief Adapt exact relocation status/path records into stable command labels and source-backed failures.
 * [TOOL-01](../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * reports preflight refusals; [COMP-04](../../../../spec/implementation/compiler.md#comp-04)
 * retains both conflicting locations rather than guessing a correction.
 */
#include "internal.h"
/** Parallel finite rows are pinned to the core enum; unsupported values never index outside their domain. Component contract: [TOOL-01](../../../../spec/implementation/tools-and-conformance.md#tool-01), [COMP-04](../../../../spec/implementation/compiler.md#comp-04). */
static const struct { const char *name; const char *message; } statuses[] = {
    {"ok","Relocation is ready"},
    {"out-of-memory","Relocation allocation failed"},
    {"invalid-path","A relocation path or endpoint setting is invalid"},
    {"io-error","A required relocation input or filesystem operation is inaccessible"},
    {"unsupported-path","A relocation path spelling or filesystem representation is unsupported"},
    {"destination-exists","The exact final destination already exists; merging or overwriting is forbidden"},
    {"descendant","The relocation endpoints overlap"},
    {"uncovered","A relocation source needs an explicit covering root"},
    {"collision","Relocation has a conflicting module identity or physical placement"},
    {"alias","Distinct relocation paths alias the same physical input"},
    {"orphan","Relocation would leave invalid physical submodule storage"},
    {"read-only","A required relocation write is not permitted"},
    {"invalid-source","A relocation source cannot be checked"},
    {"invalid-reference","A relocation reference cannot be resolved"},
    {"reference-conflict","Relocation cannot preserve the checked reference graph"},
    {"changed-input","A relocation input no longer has its planned identity or bytes"},
    {"busy","Exclusive relocation access is unavailable"},
    {"incomplete-transaction","Relocation retains pending recovery data"},
    {"invalid-journal","The relocation journal is malformed or has invalid ownership"}
};
_Static_assert(sizeof(statuses)/sizeof(*statuses) == RELOCATION_INVALID_JOURNAL+1,"Every core relocation status needs an exact command label");
/** Status labels describe the core operation, independently of later output acknowledgement. Component contract: [TOOL-01](../../../../spec/implementation/tools-and-conformance.md#tool-01), [COMP-04](../../../../spec/implementation/compiler.md#comp-04). */
const char *driver_relocation_status(RelocationStatus status) {
    return (size_t)status < sizeof(statuses)/sizeof(*statuses) ? statuses[status].name : "invalid-status";
}
/** Preserve richer compiler/catalog diagnostics; otherwise expose the exact original preflight paths and status cause. Component contract: [TOOL-01](../../../../spec/implementation/tools-and-conformance.md#tool-01), [COMP-04](../../../../spec/implementation/compiler.md#comp-04). */
void driver_relocation_preview_error(DriverRelocationPlan *plan) {
    plan->status = plan->preview.status;
    if (plan->preview.diagnostic.message != NULL) { plan->error = &plan->preview.diagnostic; return; }
    const char *message = (size_t)plan->status < sizeof(statuses)/sizeof(*statuses)
        ? statuses[plan->status].message : "The relocation core returned an invalid status";
    plan->failure = (Diagnostic){.code=plan->status == RELOCATION_OUT_OF_MEMORY ? DIAGNOSTIC_OUT_OF_MEMORY
        : plan->status == RELOCATION_IO_ERROR || plan->status == RELOCATION_READ_ONLY ? DIAGNOSTIC_IO_FAILURE : DIAGNOSTIC_INVALID_PATH,
        .message=message,.primary={.path=plan->preview.path}};
    if (plan->preview.related_path != NULL) {
        plan->failure_related = (DiagnosticRelated){.location={.path=plan->preview.related_path},.role="conflicting relocation path"};
        plan->failure.related = &plan->failure_related;
        plan->failure.related_count = 1;
    }
    plan->error = &plan->failure;
}
