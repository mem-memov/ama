/** @file
 * @brief Preserve first application errors and borrowed dependency diagnostics without reporting allocation.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves original sources.
 */
#include "internal.h"

/** A definition is an original callee formal, never a synthesized source span.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) retains declaration context.
 */
bool static_application_fail(StaticApplicationPlan *plan, StaticApplicationStatus status,
    SourceSpan site, const char *message, const StaticSymbol *formal) {
    if (plan->status != STATIC_APPLY_OK) { return false; }
    plan->status = status;
    plan->error = (Diagnostic){.code = status == STATIC_APPLY_OUT_OF_MEMORY ? DIAGNOSTIC_OUT_OF_MEMORY :
        DIAGNOSTIC_TYPE_MISMATCH, .message = message, .primary = {.span = site}};
    if (formal != NULL) {
        plan->related = (DiagnosticRelated){.location = {.span = formal->site},
            .role = "original static formal", .name = formal->owner->definition->source->logical_path};
        plan->error.related = &plan->related;
        plan->error.related_count = 1;
    }
    return false;
}

/** Borrowed errors remain valid through the declared owner destruction order.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) keeps the first error.
 */
bool static_application_dependencies(StaticApplicationPlan *plan) {
    if (plan->status != STATIC_APPLY_OK) { return false; }
    if (plan->terms->status != STATIC_TERM_OK) {
        plan->status = plan->terms->status == STATIC_TERM_OUT_OF_MEMORY ? STATIC_APPLY_OUT_OF_MEMORY : STATIC_APPLY_INVALID;
        plan->error = plan->terms->error;
    } else if (plan->solver->status != STATIC_CONSTRAINT_OK) {
        plan->status = plan->solver->status == STATIC_CONSTRAINT_OUT_OF_MEMORY ? STATIC_APPLY_OUT_OF_MEMORY :
            plan->solver->status == STATIC_CONSTRAINT_INVALID ? STATIC_APPLY_INVALID :
            plan->solver->status == STATIC_CONSTRAINT_UNRESOLVED ? STATIC_APPLY_UNRESOLVED : STATIC_APPLY_CONFLICT;
        plan->error = plan->solver->error;
    } else if (plan->bindings->status != BINDING_OK) {
        plan->status = plan->bindings->status == BINDING_OUT_OF_MEMORY ? STATIC_APPLY_OUT_OF_MEMORY : STATIC_APPLY_CONFLICT;
        plan->error = plan->bindings->error;
    }
    return plan->status == STATIC_APPLY_OK;
}
