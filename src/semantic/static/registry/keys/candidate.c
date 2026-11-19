/** @file
 * @brief Intern complete canonical owner identities before any source body expansion.
 * [TPL-05](../../../../../spec/language/templates.md#tpl-05) retains exact normalized bindings.
 * [NAME-02](../../../../../spec/implementation/compiler.md#name-02) refuses unequal full-key collisions.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns unpublished records.
 */
#include <stdint.h>
#include "../internal.h"

/** Encoding status retains exact collision evidence in the borrowed name owner;
 * no suffix, partial key or guessed category can replace a refused identity.
 * [NAME-02](../../../../../spec/implementation/compiler.md#name-02).
 */
static bool name_failure(StaticRegistryPlan *plan, StaticKeyStatus status, SourceSpan site) {
    if (status == STATIC_KEY_OUT_OF_MEMORY || status == STATIC_KEY_LIMIT) {
        return static_registry_fail(plan, STATIC_REGISTRY_OUT_OF_MEMORY, site,
            "Specialization identity exceeds available storage");
    }
    if (status == STATIC_KEY_COLLISION) {
        return static_registry_fail(plan, STATIC_REGISTRY_COLLISION, site,
            "Distinct specialization identities have the same generated-name digest");
    }
    return static_registry_fail(plan, STATIC_REGISTRY_INVALID, site, "Invalid closed specialization identity");
}

/** Canonical environments compare complete normalized metadata, never a hash or
 * source name alone. Publish only after complete name registration succeeds.
 * [TPL-05](../../../../../spec/language/templates.md#tpl-05) shares metadata without runtime state.
 */
bool static_registry_candidate(StaticRegistryPlan *plan, const StaticTermEnvironment *key,
    SourceSpan site, StaticRegistryCandidate **output) {
    for (StaticRegistryCandidate *candidate = plan->first_candidate; candidate; candidate = candidate->next) {
        if (candidate->key == key) { *output = candidate; return true; }
    }
    if (plan->candidate_count == SIZE_MAX) {
        return static_registry_fail(plan, STATIC_REGISTRY_OUT_OF_MEMORY, site,
            "Specialization candidate count exceeds representable storage");
    }
    StaticRegistryCandidate *candidate = static_registry_allocate(plan, sizeof(*candidate));
    if (candidate == NULL) { return false; }
    const IdentityName *name;
    StaticKeyStatus status = static_key_register(plan->guards->applications->terms, key, plan->names, &name);
    if (status != STATIC_KEY_OK) { return name_failure(plan, status, site); }
    *candidate = (StaticRegistryCandidate){.key = key, .name = name, .ordinal = plan->candidate_count,
        .state = STATIC_CANDIDATE_NEW, .next = plan->first_candidate};
    plan->first_candidate = candidate;
    plan->candidate_count++;
    *output = candidate;
    return true;
}
