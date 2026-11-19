/** @file
 * @brief Iterate only genuine finite refinements, then distinguish satisfaction from pending closure.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) rejects heuristic pass budgets.
 */
#include "../internal.h"

/** Each repeated pass requires a new variable binding or distinct structural
 * pair; a stable unresolved pass returns Pending rather than guessing an anchor.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) requires exact fixed points.
 */
bool static_constraint_solve(StaticConstraintSolver *solver, StaticConstraintOutcome *output) {
    if (solver->status != STATIC_CONSTRAINT_OK) { return false; }
    if (output == NULL) {
        return static_constraint_failure(solver, STATIC_CONSTRAINT_INVALID, (SourceSpan){0},
            "Static solve requires an outcome receiver", NULL, NULL);
    }
    if (!solver->dirty) { *output = solver->outcome; return true; }
    bool changed, pending;
    do {
        changed = false;
        pending = false;
        solver->pending_left = NULL;
        solver->pending_right = NULL;
        solver->pending_site = (SourceSpan){0};
        for (const StaticConstraintEquation *equation = solver->first_equation; equation; equation = equation->next) {
            if (!static_constraint_process(solver, equation, &changed, &pending)) {
                static_constraint_equation_context(solver, equation);
                return false;
            }
        }
        if (!static_constraint_classes(solver, &pending)) { return false; }
    } while (changed);
    for (const StaticConstraintRequirement *item = solver->first_requirement; item; item = item->next) {
        bool closed;
        if (!static_constraint_closed(solver, item->term, &closed)) { return false; }
        if (!closed) { pending = true; }
    }
    solver->outcome = pending ? STATIC_CONSTRAINT_PENDING : STATIC_CONSTRAINT_COMPLETE;
    solver->dirty = false;
    *output = solver->outcome;
    return true;
}
