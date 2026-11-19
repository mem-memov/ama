/** @file
 * @brief Preserve source-located first failures across lexical and static semantic ownership.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves actual diagnostic provenance.
 */
#include "internal.h"

/** Failure does not allocate and never overwrites an earlier source error.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) retains first errors.
 */
bool static_connection_fail(StaticConnectionPlan *plan, StaticConnectionStatus status, SourceSpan site, const char *message) {
    if (plan->status == STATIC_CONNECTION_OK) {
        plan->status = status;
        plan->error = (Diagnostic){.code = status == STATIC_CONNECTION_OUT_OF_MEMORY ? DIAGNOSTIC_OUT_OF_MEMORY : DIAGNOSTIC_TYPE_MISMATCH,
            .message = message, .primary = {.span = site}};
    }
    return false;
}

/** Constant/static/source owners outlive borrowed error payloads. Healthy Pending
 * remains healthy; only actual dependency refusal changes this owner's status.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves composed failure provenance.
 */
bool static_connection_dependencies(StaticConnectionPlan *plan) {
    if (plan->status != STATIC_CONNECTION_OK) { return false; }
    if (static_constant_dependencies(plan->constants)) { return true; }
    switch (plan->constants->status) {
        case STATIC_CONSTANT_OUT_OF_MEMORY: plan->status = STATIC_CONNECTION_OUT_OF_MEMORY; break;
        case STATIC_CONSTANT_INVALID: plan->status = STATIC_CONNECTION_INVALID; break;
        case STATIC_CONSTANT_UNRESOLVED: plan->status = STATIC_CONNECTION_UNRESOLVED; break;
        default: plan->status = STATIC_CONNECTION_CONFLICT; break;
    }
    plan->error = plan->constants->error;
    return false;
}
