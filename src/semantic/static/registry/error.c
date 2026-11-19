/** @file
 * @brief Preserve first registry diagnostics and borrowed guard failures without reporting allocation.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) retains destructible failed owners.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves original source evidence.
 */
#include "internal.h"

/** A sticky plan failure invalidates all registry certificates, preserving keys
 * and names solely for diagnostics and cleanup, never admission.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03).
 */
static void fail_candidates(StaticRegistryPlan *plan) {
    for (StaticRegistryCandidate *item = plan->first_candidate; item; item = item->next) {
        item->state = STATIC_CANDIDATE_FAILED;
    }
}

/** A later error cannot replace the diagnostic that stopped preparation.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04).
 */
bool static_registry_fail(StaticRegistryPlan *plan, StaticRegistryStatus status,
    SourceSpan site, const char *message) {
    if (plan->status == STATIC_REGISTRY_OK) {
        plan->status = status;
        plan->error = (Diagnostic){.code = status == STATIC_REGISTRY_OUT_OF_MEMORY ? DIAGNOSTIC_OUT_OF_MEMORY :
            status == STATIC_REGISTRY_COLLISION ? DIAGNOSTIC_IDENTITY_COLLISION : DIAGNOSTIC_TYPE_MISMATCH,
            .message = message, .primary = {.span = site}};
    }
    fail_candidates(plan);
    return false;
}

/** Guard propagation already retains the original application/solver witness.
 * Registry records never copy or own those longer-lived diagnostic allocations.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04).
 */
bool static_registry_dependencies(StaticRegistryPlan *plan) {
    if (plan->status != STATIC_REGISTRY_OK) { return false; }
    if (static_guard_dependencies(plan->guards)) { return true; }
    switch (plan->guards->status) {
        case STATIC_GUARD_OUT_OF_MEMORY: plan->status = STATIC_REGISTRY_OUT_OF_MEMORY; break;
        case STATIC_GUARD_CONFLICT: plan->status = STATIC_REGISTRY_CONFLICT; break;
        default: plan->status = STATIC_REGISTRY_INVALID; break;
    }
    plan->error = plan->guards->error;
    fail_candidates(plan);
    return false;
}
