/** @file
 * @brief Retain every actual ordinary use while selecting exact-key representatives for finite body scheduling.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) preserves incoming guards and shares closed-key expansion.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) preserves original scoped variables.
 */
#include <stdint.h>
#include "../internal.h"

/** Publish an entry only after its original actual context has a typed scope;
 * failed preparation leaves every child allocation owned by its existing arena.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) retains these actual incoming identities independently of shared body work.
 */
static bool retain(StaticClosurePlan *plan, const StaticResultContext *actual, bool *changed) {
    for (StaticClosureEntry *item = plan->first; item; item = item->next) {
        if (item->context == actual) { return true; }
    }
    if (plan->count == SIZE_MAX) {
        return static_closure_fail(plan, STATIC_CLOSURE_OUT_OF_MEMORY, actual->creation->site,
            "Required specialization worklist count exceeds representable storage");
    }
    const StaticTypedScope *scope;
    if (!static_connection_context(plan->connections, actual, &scope)) {
        (void)static_closure_dependencies(plan); return false;
    }
    StaticClosureEntry *item = static_closure_allocate(plan, sizeof(*item));
    if (!item) { return false; }
    *item = (StaticClosureEntry){.context = actual, .scope = scope};
    if (plan->last) { plan->last->next = item; }
    else { plan->first = item; }
    plan->last = item;
    plan->count++;
    *changed = true;
    return true;
}

/** Source names never establish equivalence. Closed keys share candidates;
 * before closure only the exact same environment can share source variables.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) retains these actual incoming identities independently of shared body work.
 */
static StaticClosureEntry *representative(StaticClosurePlan *plan, StaticClosureEntry *entry) {
    const StaticRegistryCandidate *candidate = entry->context->registry_use->candidate;
    for (StaticClosureEntry *item = plan->first; item != entry; item = item->next) {
        if ((candidate && candidate == item->context->registry_use->candidate) ||
            item->scope->environment == entry->scope->environment) { return item->representative; }
    }
    return entry;
}

/** Refresh guards before representative selection, then retain finite actual
 * contexts and canonical scope equalities. This operation neither walks bodies
 * nor claims local candidate checking; T185 owns that separate certificate.
 * Collection expires any previous whole-worklist certificate, even when it
 * discovers no new work. Only final resolution publishes that certificate.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) checks warm-cache paths independently of body sharing.
 */
bool static_closure_collect(StaticClosurePlan *plan, bool *output) {
    plan->closed = false;
    if (!static_closure_dependencies(plan)) { return false; }
    if (!output) {
        return static_closure_fail(plan, STATIC_CLOSURE_INVALID, (SourceSpan){0}, "Missing required-context progress receiver");
    }
    StaticResultPlan *results = plan->connections->constants->publications->results;
    StaticRegistryPlan *registry = results->registry;
    StaticRegistryKeyState keys;
    size_t equations = registry->guards->applications->solver->equation_count;
    if (!static_registry_refresh_keys(registry, &keys)) {
        (void)static_closure_dependencies(plan); return false;
    }
    bool changed = false;
    for (const StaticResultContext *actual = results->first_context; actual; actual = actual->next) {
        if (actual->creation->owner->environment->definition->definition->syntax.sections.kind == MODULE_STATIC) { continue; }
        if (!retain(plan, actual, &changed)) { return false; }
    }
    for (StaticClosureEntry *entry = plan->first; entry; entry = entry->next) {
        if (!static_registry_scope_environment(registry, entry->context->registry_use, &keys)) {
            (void)static_closure_dependencies(plan); return false;
        }
        StaticClosureEntry *selected = representative(plan, entry);
        changed |= entry->representative != selected;
        entry->representative = selected;
    }
    changed |= equations != registry->guards->applications->solver->equation_count;
    *output = changed;
    return true;
}
