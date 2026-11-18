/** @file
 * @brief Preserve original static publication failures without allocating diagnostic state.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves first-error provenance.
 */
#include "internal.h"

/** Preserve the first error; unresolved producer evidence differs from conflict.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) rejects missing inputs.
 */
bool static_publication_fail(StaticPublicationPlan *plan, StaticPublicationStatus status,
    SourceSpan site, const char *message) {
    if (plan->status != STATIC_PUBLICATION_OK) { return false; }
    plan->status = status;
    plan->error = (Diagnostic){.code = status == STATIC_PUBLICATION_OUT_OF_MEMORY ? DIAGNOSTIC_OUT_OF_MEMORY :
        DIAGNOSTIC_TYPE_MISMATCH, .message = message, .primary = {.span = site}};
    return false;
}

/** Result diagnostics borrow their longer-lived original source/value owners.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves diagnostic lifetime.
 */
bool static_publication_dependencies(StaticPublicationPlan *plan) {
    if (plan->status != STATIC_PUBLICATION_OK) { return false; }
    if (static_result_dependencies(plan->results)) { return true; }
    plan->status = plan->results->status == STATIC_RESULT_OUT_OF_MEMORY ? STATIC_PUBLICATION_OUT_OF_MEMORY :
        plan->results->status == STATIC_RESULT_INVALID ? STATIC_PUBLICATION_INVALID : STATIC_PUBLICATION_CONFLICT;
    plan->error = plan->results->error;
    return false;
}
