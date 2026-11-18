/** @file
 * @brief Convert stable required unknowns into underdetermination diagnostics at explicit finalization.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) forbids default solutions.
 */
#include "../internal.h"

/** Incremental Pending remains healthy until the caller explicitly states that
 * all independent evidence is present. [TPL-10](../../../../../spec/language/templates.md#tpl-10).
 */
bool static_constraint_finish(StaticConstraintSolver *solver) {
    StaticConstraintOutcome outcome;
    if (!static_constraint_solve(solver, &outcome)) { return false; }
    if (outcome == STATIC_CONSTRAINT_COMPLETE) { return true; }
    for (const StaticConstraintRequirement *item = solver->first_requirement; item; item = item->next) {
        bool closed;
        if (!static_constraint_closed(solver, item->term, &closed)) { return false; }
        if (!closed) {
            return static_constraint_failure(solver, STATIC_CONSTRAINT_UNRESOLVED, item->site,
                "Required static value is not uniquely determined by the available constraints",
                item->term, static_constraint_root(solver, item->term));
        }
    }
    return static_constraint_failure(solver, STATIC_CONSTRAINT_UNRESOLVED, solver->pending_site,
        "Required constructor signature metadata remains unresolved",
        solver->pending_left, solver->pending_right);
}
