/** @file
 * @brief Admit one exact original domain and immutable declared inputs before any unit or empty selection.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * forbids joining independent input observations by matching names alone;
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * keeps captured consistency separate from later filesystem freshness.
 */
#include "expansion.h"
/** Pointer identity ties compiler proof to the exact complete frozen input view. */
static bool admitted(const SourceExpansion *expansion) {
    const SourceRootCapture *capture = expansion->capture;
    const SourceAnalysis *analysis = expansion->analysis;
    return capture && capture->valid && analysis && analysis->valid
        && capture->view.entries == analysis->view.entries && capture->view.count == analysis->view.count
        && capture->view.versions == analysis->view.versions && capture->view.companions == analysis->view.companions;
}
AcquisitionClosureStatus source_expansion_validate(SourceExpansion *expansion, SourceIssue *issue) {
    if (!issue) { return ACQUISITION_CLOSURE_INVALID; }
    *issue = (SourceIssue){.message = "Structural expansion lacks an admitted original source domain"};
    if (!expansion) { return ACQUISITION_CLOSURE_INVALID; }
    expansion->failed_path = NULL;
    if (!admitted(expansion)) { return ACQUISITION_CLOSURE_INVALID; }
    if (!expansion->declarations_checked) {
        if (!source_declared_groups_valid(expansion->capture, expansion->declared)) {
            issue->message = "Declared inputs lack exact original ownership or usable captures";
            return ACQUISITION_CLOSURE_INVALID;
        }
        if (!source_declared_groups_match_original(expansion->capture, expansion->declared, &expansion->failed_path)) {
            issue->message = "Declared input observation contradicts the original capture";
            return ACQUISITION_CLOSURE_CONFLICT;
        }
        expansion->declarations_checked = true;
    }
    *issue = (SourceIssue){0}; return ACQUISITION_CLOSURE_OK;
}
