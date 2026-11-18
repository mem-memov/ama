/** @file
 * @brief Resolve selected closed values without confusing query success with executable admission.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) retains independent required obligations.
 */
#include "internal.h"

/** Global consistency is checked first; only the selected value's closure
 * controls this query. Other producer/admission obligations remain independent.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) separates constraint solving from expansion.
 */
bool static_constraint_value(StaticConstraintSolver *solver, const StaticTerm *term,
    const StaticTerm **output, StaticConstraintOutcome *outcome) {
    if (solver->status != STATIC_CONSTRAINT_OK) { return false; }
    if (output == NULL || outcome == NULL || !static_term_has_value(solver->terms, term)) {
        return static_constraint_failure(solver, STATIC_CONSTRAINT_INVALID, (SourceSpan){0},
            "Static value query requires owned input and output receivers", NULL, NULL);
    }
    StaticConstraintOutcome graph_outcome;
    if (!static_constraint_solve(solver, &graph_outcome)) { return false; }
    bool closed;
    if (!static_constraint_closed(solver, term, &closed)) { return false; }
    if (!closed) { *outcome = STATIC_CONSTRAINT_PENDING; return true; }
    const StaticTerm *value;
    if (!static_constraint_materialize(solver, term, &value)) { return false; }
    if (!static_constraint_solve(solver, &graph_outcome)) { return false; }
    *output = value;
    *outcome = STATIC_CONSTRAINT_COMPLETE;
    return true;
}
