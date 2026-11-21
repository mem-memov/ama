/** @file
 * @brief Close finite validation groups by exact backwards propagation of pending obligations.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) admits closed cycles only with all checks.
 */
#include "../internal.h"

/** Orchestrate demanded-path retention and key refresh before the nonallocating
 * group check. Path ancestry is traversed iteratively, never recursively.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) retains every required incoming path.
 */
bool static_registry_refresh_checks(StaticRegistryPlan *plan, StaticRegistryCheckState *output) {
    if (!static_registry_dependencies(plan)) { return false; }
    if (!output) {
        return static_registry_fail(plan, STATIC_REGISTRY_INVALID, (SourceSpan){0}, "Missing specialization check-closure receiver");
    }
    /* Actual demanded paths retain all incoming validation uses. Annotation
     * predecessors participate only in lexical creation reachability, never
     * in ordinary admission or executable contexts (TPL-11). */
    for (const StaticCreationPath *path = plan->guards->first_path; path; path = path->next) {
        if (path->source_only) { continue; }
        StaticRegistryUse *use;
        StaticRegistryKeyState state;
        if (!static_registry_use(plan, path, &use, &state)) { return false; }
    }
    StaticRegistryKeyState keys;
    if (!static_registry_refresh_keys(plan, &keys)) { return false; }
    static_registry_close_checks(plan, keys, output);
    return true;
}
