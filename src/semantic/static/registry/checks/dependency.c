/** @file
 * @brief Retain validation obligations without inventing constructor-creation edges.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) keeps every outgoing cached dependency.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns partial preparation.
 */
#include <stdint.h>
#include "../internal.h"

/** A new obligation may reopen a previously checked finite group.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) forbids cache bypass.
 */
void static_registry_invalidate_checks(StaticRegistryPlan *plan) {
    for (StaticRegistryCandidate *item = plan->first_candidate; item; item = item->next) {
        if (item->state == STATIC_CANDIDATE_CHECKED) { item->state = STATIC_CANDIDATE_PENDING; }
    }
}

/** Publish only after endpoint validation and complete record preparation.
 * Source span belongs to the caller's checked phase, not fabricated ancestry.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) separates validation and creation.
 */
bool static_registry_depend(StaticRegistryPlan *plan, StaticRegistryUse *owner,
    StaticRegistryUse *target, SourceSpan site) {
    if (!static_registry_dependencies(plan)) { return false; }
    if (!static_registry_has_use(plan, owner) || !static_registry_has_use(plan, target)) {
        return static_registry_fail(plan, STATIC_REGISTRY_INVALID, site, "Invalid specialization validation dependency");
    }
    for (StaticRegistryDependency *item = plan->first_dependency; item; item = item->next) {
        if (item->owner == owner && item->target == target) { return true; }
    }
    if (plan->dependency_count == SIZE_MAX) {
        return static_registry_fail(plan, STATIC_REGISTRY_OUT_OF_MEMORY, site,
            "Specialization dependency count exceeds representable storage");
    }
    StaticRegistryDependency *item = static_registry_allocate(plan, sizeof(*item));
    if (!item) { return false; }
    *item = (StaticRegistryDependency){.next = plan->first_dependency, .owner = owner, .target = target, .site = site};
    plan->first_dependency = item;
    plan->dependency_count++;
    static_registry_invalidate_checks(plan);
    return true;
}
