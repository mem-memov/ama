/** @file
 * @brief Reuse independently checked closed definition values without bypassing incoming path checks.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) validates cached dependencies.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves derived result identity.
 */
#include "../internal.h"

/** A proof from the same actual creation context has already faced its incoming
 * ancestry. Foreign proof reuse additionally requires every actual ancestor key
 * closed, so the refreshed graph can decide all comparisons against its cached
 * descendants. Otherwise replay the RHS in this context; cached value equations
 * remain useful independent evidence but cannot certify Pending ancestry.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) preserves cold/warm path checks.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) separates value evidence from readiness.
 */
static const StaticResultUse *proof(const StaticResultDefinitionUse *use, const StaticResultCache *cached) {
    const StaticResultUse *rhs = cached->rhs;
    if (use->rhs != NULL && !use->active && !use->rhs->active &&
        use->outcome == STATIC_RESULT_READY && use->rhs->outcome == STATIC_RESULT_READY && use->rhs->closed &&
        use->rhs->source == use->definition->source && use->rhs->context == use->context) {
        rhs = use->rhs;
    }
    if (rhs->context == use->context) { return rhs; }
    for (const StaticCreationPath *path = use->context->creation; path; path = path->parent) {
        if (path->vertex == NULL) { return NULL; }
    }
    return rhs;
}

/** A closed owner/symbol pair denotes one checked static definition. Its current
 * scoped variable receives equality, never rebinding; new evidence is solved and
 * guard paths refreshed before reporting a cache hit.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves exact equality.
 */
bool static_result_cache_read(StaticResultPlan *plan, StaticResultDefinitionUse *use, bool *output) {
    if (!static_result_dependencies(plan)) { return false; }
    if (output == NULL || !static_result_has_definition_use(plan, use)) {
        return static_result_fail(plan, STATIC_RESULT_INVALID, (SourceSpan){0}, "Invalid static result cache-use identity");
    }
    StaticGuardClosure closure;
    if (!static_guard_refresh(plan->guards, &closure)) { (void)static_result_dependencies(plan); return false; }
    const StaticTerm *owner = use->path->node->concrete;
    if (owner != NULL) {
        for (const StaticResultCache *cached = plan->first_cache; cached; cached = cached->next) {
            if (cached->owner != owner || cached->symbol != use->definition->symbol) { continue; }
            StaticConstraintSolver *solver = plan->guards->applications->solver;
            StaticConstraintOutcome solved;
            if (!static_constraint_equate(solver, use->definition->value, cached->value, use->path->site) ||
                !static_constraint_solve(solver, &solved) || !static_guard_refresh(plan->guards, &closure)) {
                (void)static_result_dependencies(plan); return false;
            }
            const StaticResultUse *checked = proof(use, cached);
            if (checked == NULL) { *output = false; return true; }
            use->cache_hit = true;
            use->checked_rhs = checked;
            use->outcome = STATIC_RESULT_READY;
            *output = true;
            return true;
        }
    }
    *output = false;
    return true;
}
