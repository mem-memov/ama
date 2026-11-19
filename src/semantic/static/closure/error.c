/** @file
 * @brief Preserve first worklist and borrowed semantic diagnostics through failed closure preparation.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves original source provenance.
 */
#include "internal.h"

/** Invalidate the aggregate certificate on refusal while retaining existing
 * immutable Type descriptors and first evidence. A failed owner is destructible
 * only. [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) prevents
 * treating its previous completion flag as current usable phase output.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves this first diagnostic and its original source lifetime.
 */
bool static_closure_fail(StaticClosurePlan *plan, StaticClosureStatus status, SourceSpan site, const char *message) {
    plan->closed = false;
    if (plan->status == STATIC_CLOSURE_OK) {
        plan->status = status;
        plan->error = (Diagnostic){.code = status == STATIC_CLOSURE_OUT_OF_MEMORY ? DIAGNOSTIC_OUT_OF_MEMORY : DIAGNOSTIC_TYPE_MISMATCH,
            .message = message, .primary = {.span = site}};
    }
    return false;
}

/** Borrowed-owner failure also expires aggregate closure. Existing descriptor
 * storage remains owned for diagnostic rendering and teardown; no success proof
 * survives a failed dependency. Borrowed owners outlive related-span arrays.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) bounds failed ownership.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves this first diagnostic and its original source lifetime.
 */
bool static_closure_dependencies(StaticClosurePlan *plan) {
    if (plan->status != STATIC_CLOSURE_OK) { return false; }
    if (static_connection_dependencies(plan->connections)) { return true; }
    plan->closed = false;
    switch (plan->connections->status) {
        case STATIC_CONNECTION_INVALID: plan->status = STATIC_CLOSURE_INVALID; break;
        case STATIC_CONNECTION_OUT_OF_MEMORY: plan->status = STATIC_CLOSURE_OUT_OF_MEMORY; break;
        case STATIC_CONNECTION_UNRESOLVED: plan->status = STATIC_CLOSURE_UNRESOLVED; break;
        default: plan->status = STATIC_CLOSURE_CONFLICT; break;
    }
    plan->error = plan->connections->error;
    return false;
}
