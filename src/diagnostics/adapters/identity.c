#include "adapters.h"

bool diagnostic_from_identity_collision(const IdentityName *previous,
    const char *requested_name, SourceSpan requested_span,
    DiagnosticRelated *related, Diagnostic *output) {
    if (previous == NULL || requested_name == NULL || related == NULL || output == NULL) {
        return false;
    }
    DiagnosticRelated note = {{previous->declaration, NULL},
        "previous identity with the same digest", previous->source_name};
    Diagnostic diagnostic = {DIAGNOSTIC_IDENTITY_COLLISION,
        "Distinct qualified identities have the same internal digest", requested_name,
        {requested_span, NULL}, "Full encoded identities differ despite an equal SHA-256 digest",
        NULL, "Rename one conflicting module or declaration and recompile", &note, 1};
    if (!diagnostic_valid(&diagnostic)) {
        return false;
    }
    *related = note;
    diagnostic.related = related;
    *output = diagnostic;
    return true;
}
