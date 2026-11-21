/**
 * @file
 * @brief Preserve a strict INI record's actual source interval on refusal.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) requires
 * source-located explanations; [PKG-03](../../../../spec/project/packages.md#pkg-03)
 * does not permit repairing or silently trimming malformed records.
 */
#include "internal.h"

bool configuration_record_failure(ConfigurationIssue *issue, SourceSpan span,
    const char *message, const char *expected) {
    *issue = (ConfigurationIssue){span, message, expected};
    return false;
}
