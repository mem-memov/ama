/** @file
 * @brief Propagate source-backed static result diagnostics without allocating during failure.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves original locations.
 */
#include "internal.h"

/** A failed owner is destructible only; later calls preserve its first error.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves actual causes.
 */
bool static_result_fail(StaticResultPlan *plan, StaticResultStatus status, SourceSpan site, const char *message) {
    if (plan->status != STATIC_RESULT_OK) { return false; }
    plan->status = status;
    plan->error = (Diagnostic){.code = status == STATIC_RESULT_OUT_OF_MEMORY ? DIAGNOSTIC_OUT_OF_MEMORY :
        DIAGNOSTIC_TYPE_MISMATCH, .message = message, .primary = {.span = site}};
    return false;
}

/** Borrowed reports remain valid because guards and their dependencies outlive
 * this plan. A pending healthy dependency is never changed into an error.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves diagnostic ownership.
 */
bool static_result_dependencies(StaticResultPlan *plan) {
    if (plan->status != STATIC_RESULT_OK) { return false; }
    if (plan->registry->status != STATIC_REGISTRY_OK) {
        plan->status = plan->registry->status == STATIC_REGISTRY_OUT_OF_MEMORY ? STATIC_RESULT_OUT_OF_MEMORY :
            plan->registry->status == STATIC_REGISTRY_INVALID ? STATIC_RESULT_INVALID : STATIC_RESULT_CONFLICT;
        plan->error = plan->registry->error;
        return false;
    }
    if (static_guard_dependencies(plan->guards)) { return true; }
    StaticGuardStatus status = plan->guards->status;
    plan->status = status == STATIC_GUARD_OUT_OF_MEMORY ? STATIC_RESULT_OUT_OF_MEMORY :
        status == STATIC_GUARD_INVALID ? STATIC_RESULT_INVALID : STATIC_RESULT_CONFLICT;
    plan->error = plan->guards->error;
    return false;
}
