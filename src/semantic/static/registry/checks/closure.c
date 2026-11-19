/** @file
 * @brief Compute the exact finite group certificate after actual use keys and guards refresh.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) preserves all reachable local obligations.
 */
#include "../internal.h"

/** Greatest fixed point starts with locally complete candidates, then removes
 * owners reaching any incomplete target. Each changing pass removes at least
 * one candidate; at most V changes occur. O(V + (V+1)E) time, O(1) extra space,
 * excluding underlying key/guard normalization. No recursive execution occurs.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) includes every outgoing branch.
 */
void static_registry_close_checks(StaticRegistryPlan *plan, StaticRegistryKeyState keys,
    StaticRegistryCheckState *output) {
    StaticRegistryCheckState closure = keys == STATIC_REGISTRY_KEY_CLOSED ? STATIC_REGISTRY_CHECKS_CLOSED : STATIC_REGISTRY_CHECKS_PENDING;
    for (StaticRegistryCandidate *item = plan->first_candidate; item; item = item->next) {
        item->state = item->local_complete ? STATIC_CANDIDATE_CHECKED : STATIC_CANDIDATE_PENDING;
    }
    bool changed;
    do {
        changed = false;
        for (const StaticRegistryDependency *edge = plan->first_dependency; edge; edge = edge->next) {
            StaticRegistryCandidate *owner = edge->owner->candidate;
            const StaticRegistryCandidate *target = edge->target->candidate;
            if (owner && owner->state == STATIC_CANDIDATE_CHECKED &&
                (!target || target->state != STATIC_CANDIDATE_CHECKED)) {
                owner->state = STATIC_CANDIDATE_PENDING;
                changed = true;
            }
        }
    } while (changed);
    for (const StaticRegistryCandidate *item = plan->first_candidate; item; item = item->next) {
        if (item->state != STATIC_CANDIDATE_CHECKED) { closure = STATIC_REGISTRY_CHECKS_PENDING; }
    }
    *output = closure;
}
