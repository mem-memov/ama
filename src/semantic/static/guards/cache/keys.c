/** @file
 * @brief Normalize symbolic creation owners into exact complete cache vertices.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) preserves concrete path identity.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) retains pending owner metadata.
 */
#include "../internal.h"

/** All registered frames retain their lexical key even when equivalent complete
 * values now share a vertex. Pending metadata can close during another query;
 * the outer pass repeats only for actual newly derived metadata facts.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves monotone inference.
 */
bool static_guard_creation_keys(StaticGuardPlan *plan, StaticGuardClosure *closure) {
    *closure = STATIC_GUARD_CLOSED;
    for (StaticCreationPath *path = plan->first_path; path; path = path->next) {
        const StaticTermEnvironment *value;
        StaticConstraintOutcome outcome;
        if (!static_guard_owner_key(plan, path->owner, &value, &outcome)) {
            (void)static_guard_dependencies(plan); return false;
        }
        if (outcome == STATIC_CONSTRAINT_PENDING) { *closure = STATIC_GUARD_PENDING; continue; }
        if (path->vertex != NULL) {
            if (path->vertex->key != value) {
                return static_guard_fail(plan, STATIC_GUARD_CONFLICT, path->site,
                    "A complete creation key changed after prior normalization");
            }
            continue;
        }
        if (!static_guard_vertex(plan, value, &path->vertex)) { return false; }
    }
    return true;
}
