/** @file
 * @brief Cache only independently computed closed static definitions while preserving every producer input.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) requires complete static producers.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) rechecks closed-key provenance.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns failed preparation.
 */
#include <stdint.h>
#include "../internal.h"

/** Owner closure uses the full compiler-only producer map. A named producer
 * RHS has no own value; its checked closed flag includes every input and selected
 * result. Pair RHS metadata retains a separate ordinary value to compare.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves unused input closure.
 */
static bool values(StaticResultPlan *plan, const StaticResultDefinitionUse *use,
    const StaticTermEnvironment **owner, const StaticTerm **rhs, const StaticTerm **value, bool *closed) {
    *closed = false;
    StaticRegistryUse *registered;
    StaticRegistryKeyState keys;
    if (!static_registry_use(plan->registry, use->context->creation, &registered, &keys)) {
        (void)static_result_dependencies(plan); return false;
    }
    if (keys == STATIC_REGISTRY_KEY_PENDING) { return true; }
    StaticConstraintSolver *solver = plan->guards->applications->solver;
    StaticConstraintOutcome outcome;
    *rhs = NULL;
    if (use->rhs->source->value) {
        if (!static_constraint_value(solver, use->rhs->source->value, rhs, &outcome)) {
            (void)static_result_dependencies(plan); return false;
        }
        if (outcome == STATIC_CONSTRAINT_PENDING) { return true; }
    }
    if (!static_constraint_value(solver, use->definition->value, value, &outcome)) {
        (void)static_result_dependencies(plan); return false;
    }
    if (outcome == STATIC_CONSTRAINT_PENDING) { return true; }
    *owner = registered->candidate->key;
    *closed = true;
    return true;
}

/** Grounded receiver evidence alone is insufficient: the actual RHS must have
 * finished static checking. Complete equal-owner definitions must return the
 * same canonical value; conflicting values fail instead of preferring the first.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves unique results.
 */
bool static_result_cache_store(StaticResultPlan *plan, StaticResultDefinitionUse *use, bool *output) {
    if (!static_result_dependencies(plan)) { return false; }
    if (output == NULL || !static_result_has_definition_use(plan, use)) {
        return static_result_fail(plan, STATIC_RESULT_INVALID, (SourceSpan){0}, "Invalid static result cache publication identity");
    }
    if (use->outcome != STATIC_RESULT_READY || use->active || use->rhs == NULL || use->rhs->active ||
        use->rhs->outcome != STATIC_RESULT_READY) { *output = false; return true; }
    if (use->rhs->source != use->definition->source) {
        return static_result_fail(plan, STATIC_RESULT_INVALID, use->definition->symbol->site,
            "Checked static definition RHS does not match its original source");
    }
    const StaticTermEnvironment *owner;
    const StaticTerm *rhs, *value;
    bool closed;
    if (!values(plan, use, &owner, &rhs, &value, &closed)) { return false; }
    if (!closed || !use->rhs->closed) { *output = false; return true; }
    if (use->definition->symbol->role == STATIC_RESULT && value != rhs) {
        return static_result_fail(plan, STATIC_RESULT_CONFLICT, use->path->site,
            "Static result value disagrees with its defining expression");
    }
    StaticGuardClosure closure;
    if (!static_guard_refresh(plan->guards, &closure)) { (void)static_result_dependencies(plan); return false; }
    for (const StaticResultCache *cached = plan->first_cache; cached; cached = cached->next) {
        if (cached->owner != owner || cached->symbol != use->definition->symbol) { continue; }
        if (cached->value != value) {
            return static_result_fail(plan, STATIC_RESULT_CONFLICT, use->path->site,
                "Static definition values disagree for one complete owner identity");
        }
        *output = true;
        return true;
    }
    if (plan->cache_count == SIZE_MAX) {
        return static_result_fail(plan, STATIC_RESULT_OUT_OF_MEMORY, use->path->site,
            "Static result cache count exceeds representable storage");
    }
    StaticResultCache *cached = static_result_allocate(plan, sizeof(*cached));
    if (cached == NULL) { return false; }
    *cached = (StaticResultCache){.owner = owner, .symbol = use->definition->symbol,
        .value = value, .rhs = use->rhs, .next = plan->first_cache};
    plan->first_cache = cached;
    plan->cache_count++;
    *output = true;
    return true;
}
