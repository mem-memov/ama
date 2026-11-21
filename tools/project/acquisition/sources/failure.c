/** @file
 * @brief Preserve original acquisition failure provenance and the previous version contract.
 * [PKG-06](../../../../spec/project/packages.md#pkg-06) reports conflicting
 * coherent sources; [COMP-04](../../../../spec/implementation/compiler.md#comp-04)
 * keeps original bounded source evidence without failure-time allocation.
 */
#include "internal.h"
AcquisitionSourcesStatus acquisition_sources_failure(AcquisitionSourcesIssue *issue, SourceSpan origin,
    AcquisitionSourcesStatus status, const char *message, const AcquisitionVersionSource *previous) {
    if (issue) {
        *issue = (AcquisitionSourcesIssue){.issue = {source_span_valid(origin) ? origin : (SourceSpan){0}, message,
            "one prepared copy or one exact URL/resolved-commit/physical-directory source per logical version"}, .previous = previous};
    }
    return status;
}
