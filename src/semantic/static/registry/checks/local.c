/** @file
 * @brief Accept explicit local phase completion independently of dependency closure.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) requires local checks before group completion.
 */
#include "../internal.h"

/** The calling phase must finish local checks and enumerate source dependencies;
 * this operation records its evidence without evaluating ordinary/static bodies.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) distinguishes identity from validation.
 */
bool static_registry_complete_local(StaticRegistryPlan *plan, StaticRegistryUse *use) {
    if (!static_registry_dependencies(plan)) { return false; }
    if (!static_registry_has_use(plan, use)) {
        return static_registry_fail(plan, STATIC_REGISTRY_INVALID, (SourceSpan){0}, "Invalid local specialization check use");
    }
    StaticRegistryUse *refreshed;
    StaticRegistryKeyState state;
    if (!static_registry_use(plan, use->path, &refreshed, &state)) { return false; }
    if (state != STATIC_REGISTRY_KEY_CLOSED) {
        return static_registry_fail(plan, STATIC_REGISTRY_INVALID, use->path->site,
            "Local specialization checks require a complete owner key");
    }
    use->candidate->local_started = true;
    use->candidate->local_complete = true;
    use->candidate->state = STATIC_CANDIDATE_PENDING;
    return true;
}
