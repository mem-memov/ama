/** @file
 * @brief Compose read-only graph preparation, durable application and lossless relocation responses without extending source syntax.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * validates the entire move before publication and keeps every borrowed diagnostic owner alive through output.
 */
#ifndef AMA_DRIVER_RELOCATION_INTERNAL_H
#define AMA_DRIVER_RELOCATION_INTERNAL_H
#include "relocation.h"
#include "tools/relocation/report/report.h"
#include "tools/relocation/recovery/recovery.h"
#include "tools/output/source/source.h"
/** Original/final graph and report owners expire in dependency order, after any borrowing transaction. */
typedef struct {
    RelocationPreview preview;
    RelocationReferences references;
    RelocationStaging staging;
    RelocationReport report;
    Diagnostic failure;
    DiagnosticRelated failure_related;
    const Diagnostic *error;
    RelocationStatus status;
} DriverRelocationPlan;
/** Initialize every partial owner without filesystem mutation. */
void driver_relocation_plan_init(DriverRelocationPlan *, SourceAllocator);
/** Destroy report/staging/references before original catalog storage; never apply or retry disk actions. */
void driver_relocation_plan_destroy(DriverRelocationPlan *);
/** Prepare exact configured scope, resolved edits and independently validated staged graph once. */
bool driver_relocation_plan_build(DriverRelocationPlan *, const DriverOptions *);
/** Stable complete status names and preview path diagnostics are exact adapters of core statuses. */
const char *driver_relocation_status(RelocationStatus);
void driver_relocation_preview_error(DriverRelocationPlan *);
/** Render copied scope/settings/module/file roles; null or incomplete reports remain explicit null. */
bool driver_relocation_report(ToolJsonWriter *, const RelocationReport *);
/** Render original exact path-prefix replacements, without treating literals/native bytes as source paths. */
bool driver_relocation_edits(ToolJsonWriter *, const RelocationReferences *);
/** Emit full outcome, report, journal and both causes while their owning operation is alive. */
bool driver_relocation_result(ToolJsonWriter *, const char *, bool, RelocationStatus, const char *, bool, bool, bool,
    const RelocationReport *, const RelocationReferences *, const char *, const Diagnostic *, const Diagnostic *);
#endif
