/**
 * @file
 * @brief Retain path-normalization status and original issue without allocating error text.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) requires source
 * attribution; [ARCH-03](../../../../spec/implementation/compiler.md#arch-03)
 * does not transfer scratch ownership merely because an operation refuses.
 */
#include "internal.h"

ModulePathStatus path_normalization_failure(SourceIssue *issue, SourceSpan span,
    ModulePathStatus status, const char *message, const char *expected) {
    *issue = (SourceIssue){span, message, expected};
    return status;
}
