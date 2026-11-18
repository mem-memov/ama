/** @file
 * @brief Preserve explicit selected-root provenance before symbolic specialization expansion.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) requires attributable roots.
 */
#include "../internal.h"

/** The caller explicitly selects a compilation root; generic formals remain
 * unknown rather than acquiring fabricated concrete defaults.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) separates root provenance from closure.
 */
bool static_creation_root(StaticGuardPlan *plan, const StaticTermEnvironment *environment,
    const StaticCreationPath **output) {
    if (!static_guard_dependencies(plan)) { return false; }
    if (output == NULL) {
        return static_guard_fail(plan, STATIC_GUARD_INVALID, (SourceSpan){0}, "Missing selected-root receiver");
    }
    const StaticGuardOwner *owner;
    if (!static_guard_owner(plan, environment, &owner)) { return false; }
    const ProjectModule *definition = environment->definition->definition;
    SourceSpan site = {definition->source->text, 0, 0};
    const StaticCreationPath *path;
    if (!static_guard_path(plan, NULL, owner, NULL, site, &path)) { return false; }
    StaticGuardClosure closure;
    if (!static_guard_refresh(plan, &closure)) { return false; }
    *output = path;
    return true;
}
