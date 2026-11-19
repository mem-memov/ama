/** @file
 * @brief Claim one local analysis per closed specialization without admitting active cycles.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) registers identity before dependency expansion.
 */
#include "../internal.h"

/** Active reuse suppresses duplicate expansion only; every incoming use still
 * passes guards, and group checking still waits for the local certificate.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) separates active and validated states.
 */
bool static_registry_begin_local(StaticRegistryPlan *plan, StaticRegistryUse *use,
    bool *should_check, StaticRegistryKeyState *state) {
    if (!static_registry_dependencies(plan)) { return false; }
    if (!should_check || !state || !static_registry_has_use(plan, use)) {
        return static_registry_fail(plan, STATIC_REGISTRY_INVALID, (SourceSpan){0}, "Invalid local specialization check claim");
    }
    StaticRegistryUse *refreshed;
    StaticRegistryKeyState keys;
    if (!static_registry_use(plan, use->path, &refreshed, &keys)) { return false; }
    bool claim = keys == STATIC_REGISTRY_KEY_CLOSED && !use->candidate->local_started;
    if (claim) {
        use->candidate->local_started = true;
        use->candidate->state = STATIC_CANDIDATE_PENDING;
    }
    *should_check = claim;
    *state = keys;
    return true;
}
