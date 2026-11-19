/** @file
 * @brief Attach caller-owned source attribution without allocating or replacing earlier error evidence.
 * [COMP-04](../../../spec/implementation/compiler.md#comp-04) retains original requiring sites and primary failures.
 */
#include <string.h>
#include "diagnostic.h"

/** Compare optional immutable prose byte-for-byte, preserving null as absence.
 * [COMP-04](../../../spec/implementation/compiler.md#comp-04) forbids replacing an earlier requiring context.
 */
static bool same_text(const char *left, const char *right) {
    return left == right || (left && right && strcmp(left, right) == 0);
}

/** An attribution frame has one exact source/prose meaning for its lifetime.
 * [COMP-04](../../../spec/implementation/compiler.md#comp-04) preserves original spans on repeated reporting.
 */
static bool same_note(DiagnosticRelated left, DiagnosticRelated right) {
    return left.location.span.source == right.location.span.source
        && left.location.span.start == right.location.span.start && left.location.span.end == right.location.span.end
        && same_text(left.location.path, right.location.path) && same_text(left.role, right.role) && same_text(left.name, right.name);
}

bool diagnostic_add_context(Diagnostic *diagnostic, DiagnosticContext *node, DiagnosticRelated related) {
    if (!node || !diagnostic_valid(diagnostic)) { return false; }
    DiagnosticContext prepared = {.related = related, .next = diagnostic->contexts};
    Diagnostic candidate = *diagnostic;
    candidate.contexts = &prepared;
    if (!diagnostic_valid(&candidate)) { return false; }
    for (const DiagnosticContext *current = diagnostic->contexts; current; current = current->next) {
        if (current == node) { return same_note(current->related, related); }
    }
    *node = prepared;
    diagnostic->contexts = node;
    return true;
}
