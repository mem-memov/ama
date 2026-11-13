/**
 * @file
 * @brief Validate diagnostic code, required prose and exact location consistency.
 *
 * [COMP-04](../../../spec/implementation/compiler.md#comp-04)
 * Reject malformed borrowed records before any renderer consumes them.
 */
#include <string.h>
#include "diagnostic.h"

/** Allow absent locations only with zero offsets; otherwise require a bounded span and exact explicit/original path agreement.
 */
static bool location_valid(DiagnosticLocation location) {
    if (location.span.source == NULL) {
        return location.span.start == 0 && location.span.end == 0;
    }
    return source_span_valid(location.span)
        && (location.path == NULL
            || strcmp(location.path, source_path(location.span.source)) == 0);
}

bool diagnostic_valid(const Diagnostic *diagnostic) {
    if (diagnostic == NULL || diagnostic->code < 0
        || diagnostic->code >= DIAGNOSTIC_CODE_COUNT
        || diagnostic->message == NULL || *diagnostic->message == 0
        || !location_valid(diagnostic->primary)
        || (diagnostic->related_count != 0 && diagnostic->related == NULL)) {
        return false;
    }
    for (size_t index = 0; index < diagnostic->related_count; index++) {
        const DiagnosticRelated *related = &diagnostic->related[index];
        if (related->role == NULL || *related->role == 0
            || !location_valid(related->location)) {
            return false;
        }
    }
    return true;
}

const char *diagnostic_code_name(DiagnosticCode code) {
    static const char *const names[] = {
        "invalid-utf8", "invalid-path", "missing-module", "ambiguous-module",
        "file-alias", "orphan-directory", "unsupported-path", "io-failure",
        "out-of-memory", "identity-collision", "syntax", "type-mismatch", "unsupported-feature"
    };
    _Static_assert(sizeof(names) / sizeof(names[0]) == DIAGNOSTIC_CODE_COUNT,
        "Every diagnostic code has one fixed machine name");
    return code >= 0 && code < DIAGNOSTIC_CODE_COUNT ? names[code] : "invalid-diagnostic";
}
