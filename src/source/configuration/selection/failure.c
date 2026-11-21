/**
 * @file
 * @brief Report exact selection-preparation failures without allocating diagnostics.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves original
 * metadata/configuration spans under [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) refusal.
 */
#include "internal.h"
DependencySelectionStatus selection_failure(SourceIssue *issue, SourceSpan span,
    DependencySelectionStatus status, const char *message, const char *expected) {
    *issue = (SourceIssue){span, message, expected};
    return status;
}
