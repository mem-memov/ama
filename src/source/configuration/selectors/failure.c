/**
 * @file
 * @brief Report selector preparation refusals without allocating or losing source sites.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves the
 * original operand; [ARCH-03](../../../../spec/implementation/compiler.md#arch-03)
 * keeps resource errors distinct from successful empty match sets.
 */
#include "internal.h"

DependencySelectorStatus selector_failure(SourceIssue *issue, SourceSpan span,
    DependencySelectorStatus status, const char *message, const char *expected) {
    *issue = (SourceIssue){span, message, expected};
    return status;
}
