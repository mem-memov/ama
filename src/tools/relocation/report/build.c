/** @file
 * @brief Capture complete relocation reporting authority before its compiler-side plan can expire.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires explicit scope, identity changes, preserved drafts and rebuild reporting.
 */
#include "internal.h"
/** Each owned section must succeed before any public report is consumable. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_report_build(RelocationReport *report, const RelocationStaging *staging) {
    if (report->attempted) { return false; }
    report->attempted = true;
    if (staging == NULL || !staging->ready) { report->status = RELOCATION_INVALID_PATH; return false; }
    if (!relocation_report_settings(report,staging) || !relocation_report_modules(report,staging)
        || !relocation_report_files(report,staging)) { return false; }
    report->rebuild_required = true;
    report->ready = true;
    return true;
}
