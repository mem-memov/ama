/** @file
 * @brief Preserve first constant type/namespace errors and longer-lived dependency diagnostics.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves original failure context.
 */
#include "internal.h"

/** Failure reporting borrows a constant message and original source span.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) avoids failure allocation.
 */
bool static_constant_fail(StaticConstantPlan *plan, StaticConstantStatus status, SourceSpan site, const char *message) {
    if (plan->status != STATIC_CONSTANT_OK) { return false; }
    plan->status = status;
    plan->error = (Diagnostic){.code = status == STATIC_CONSTANT_OUT_OF_MEMORY ? DIAGNOSTIC_OUT_OF_MEMORY :
        DIAGNOSTIC_TYPE_MISMATCH, .message = message, .primary = {.span = site}};
    return false;
}

/** Healthy symbolic publication dependencies do not choose a namespace; failures
 * preserve their borrowed original diagnostic and exact status class.
 * [CONST-03](../../../../spec/language/declarations.md#const-03) forbids namespace guessing.
 */
bool static_constant_dependencies(StaticConstantPlan *plan) {
    if (plan->status != STATIC_CONSTANT_OK) { return false; }
    if (static_publication_dependencies(plan->publications)) { return true; }
    StaticPublicationStatus status = plan->publications->status;
    plan->status = status == STATIC_PUBLICATION_OUT_OF_MEMORY ? STATIC_CONSTANT_OUT_OF_MEMORY :
        status == STATIC_PUBLICATION_INVALID ? STATIC_CONSTANT_INVALID :
        status == STATIC_PUBLICATION_UNRESOLVED ? STATIC_CONSTANT_UNRESOLVED : STATIC_CONSTANT_CONFLICT;
    plan->error = plan->publications->error;
    return false;
}
