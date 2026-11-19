/** @file
 * @brief Check complete producer input maps and selected value metadata without requiring a nonexistent producer value.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) distinguishes producer closure and returned Templates.
 */
#include "internal.h"

/** Key closure includes unused selected-input bindings. Selected results retain
 * their own actual Type/Template identities; ordinary admission remains separate.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12).
 */
bool static_result_producer_closed(StaticResultPlan *plan, StaticResultUse *use, bool *output) {
    if (!use->producer) { *output = false; return true; }
    StaticRegistryUse *registered;
    StaticRegistryKeyState keys;
    if (!static_registry_use(plan->registry, use->producer->creation, &registered, &keys)) {
        (void)static_result_dependencies(plan); return false;
    }
    bool closed = keys == STATIC_REGISTRY_KEY_CLOSED;
    StaticTermStore *terms = plan->guards->applications->terms;
    for (const BindingRequest *request = use->requests->first; request; request = request->next) {
        if (request->direction != BINDING_STATIC_OUTPUT) { continue; }
        const StaticTerm *value;
        bool selected_closed;
        if (!static_term_symbol(terms, use->producer->creation->owner->environment,
            request->slot->static_symbol, &value)) {
            (void)static_result_dependencies(plan); return false;
        }
        if (!static_result_closed(plan, value, &selected_closed)) { return false; }
        closed &= selected_closed;
    }
    *output = closed;
    return true;
}
