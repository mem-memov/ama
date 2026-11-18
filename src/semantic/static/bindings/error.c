/**
 * @file
 * @brief Keep first binding metadata failures without allocating diagnostic storage.
 *
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves
 * actual source positions even when semantic preparation cannot allocate.
 */
#include "internal.h"

/** Allocation refusal supersedes no earlier source error and needs no source
 * fabrication. Every partial snapshot remains owned by the plan arena.
 * Contract: [COMP-04](../../../../spec/implementation/compiler.md#comp-04).
 */
bool binding_memory_failure(BindingPlan *plan) {
    if (plan->status == BINDING_OK) {
        plan->status = BINDING_OUT_OF_MEMORY;
        plan->error = (Diagnostic){.code = DIAGNOSTIC_OUT_OF_MEMORY,
            .message = "Binding metadata could not allocate storage"};
    }
    return false;
}

/** Messages have constant lifetime; an absent source site is allowed only for
 * invalid compiler-supplied intrinsic/configuration metadata, never invented.
 * Contract: [COMP-04](../../../../spec/implementation/compiler.md#comp-04).
 */
bool binding_invalid(BindingPlan *plan, SourceSpan site, const char *message) {
    if (plan->status == BINDING_OK) {
        plan->status = BINDING_INVALID;
        plan->error = (Diagnostic){.code = DIAGNOSTIC_TYPE_MISMATCH,
            .message = message, .primary = {.span = site}};
    }
    return false;
}
