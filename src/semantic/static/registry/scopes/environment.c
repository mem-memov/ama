/** @file
 * @brief Canonicalize every existing scoped static variable after one owner key becomes complete.
 * [TPL-05](../../../../../spec/language/templates.md#tpl-05) forbids hidden context-dependent specialization inputs.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) preserves lexical identity across all variable roles.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) refreshes actual provenance before sharing.
 */
#include "../internal.h"

/** Snapshot the finite term count before binding can append canonical terms.
 * New counterparts are already canonical. Each original record is visited once;
 * explicit source terms created later require another scan, never a guessed role
 * list or recursive expansion. Costs are one O(N) scan plus exact interning,
 * equation and underlying key/guard costs; no temporary array is allocated.
 * [TPL-05](../../../../../spec/language/templates.md#tpl-05) equates analysis, not runtime instances.
 */
bool static_registry_scope_environment(StaticRegistryPlan *plan, StaticRegistryUse *use,
    StaticRegistryKeyState *output) {
    if (!static_registry_dependencies(plan)) { return false; }
    if (!output || !static_registry_has_use(plan, use)) {
        return static_registry_fail(plan, STATIC_REGISTRY_INVALID, (SourceSpan){0}, "Invalid static environment scope query");
    }
    StaticRegistryUse *refreshed;
    StaticRegistryKeyState state;
    if (!static_registry_use(plan, use->path, &refreshed, &state)) { return false; }
    if (state == STATIC_REGISTRY_KEY_CLOSED) {
        StaticTermStore *terms = plan->guards->applications->terms;
        size_t remaining = terms->term_count;
        for (const StaticTerm *term = terms->first; term && remaining; term = term->next, remaining--) {
            if (term->kind != STATIC_TERM_VARIABLE ||
                term->as.variable.environment != use->path->owner->environment) { continue; }
            const StaticTerm *canonical;
            if (!static_registry_bind_scope(plan, use, term, &canonical)) { return false; }
        }
    }
    *output = state;
    return true;
}
