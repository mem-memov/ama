/** @file
 * @brief Preserve first guard diagnostics without allocating during a refusal.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) retains source evidence.
 */
#include "internal.h"

/** First failure wins; previously retained full witness paths stay inspectable.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves context.
 */
bool static_guard_fail(StaticGuardPlan *plan, StaticGuardStatus status, SourceSpan site, const char *message) {
    if (plan->status != STATIC_GUARD_OK) { return false; }
    plan->status = status;
    plan->error = (Diagnostic){.code = status == STATIC_GUARD_OUT_OF_MEMORY ? DIAGNOSTIC_OUT_OF_MEMORY :
        DIAGNOSTIC_TYPE_MISMATCH, .message = message, .primary = {.span = site}};
    return false;
}

/** Borrowed diagnostics remain valid until applications and their owners expire.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves first errors.
 */
bool static_guard_dependencies(StaticGuardPlan *plan) {
    if (plan->status != STATIC_GUARD_OK) { return false; }
    if (static_application_dependencies(plan->applications)) { return true; }
    StaticApplicationStatus status = plan->applications->status;
    plan->status = status == STATIC_APPLY_OUT_OF_MEMORY ? STATIC_GUARD_OUT_OF_MEMORY :
        status == STATIC_APPLY_INVALID ? STATIC_GUARD_INVALID : STATIC_GUARD_CONFLICT;
    plan->error = plan->applications->error;
    return false;
}
