/** @file
 * @brief Expose a proven nominal head without closing captures or expanding a definition.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) forbids inferring an origin from expected operations.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) keeps interface facts distinct from expansion permission.
 */
#include "../internal.h"

/** Solving gives one acyclic representative for the supplied value. A nominal
 * representative already fixes its original constructor even if captures remain
 * unknown. An unbound variable proves no constructor and remains Pending. The
 * returned term retains its immutable original children; later queries follow
 * the solver again rather than mutating prior terms or inventing a candidate.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) separates facts from concrete closure.
 */
bool static_constraint_nominal(StaticConstraintSolver *solver, const StaticTerm *value,
    const StaticTerm **output, StaticConstraintOutcome *outcome) {
    if (solver->status != STATIC_CONSTRAINT_OK) { return false; }
    if (!output || !outcome || !static_term_has_value(solver->terms, value)) {
        return static_constraint_failure(solver, STATIC_CONSTRAINT_INVALID, (SourceSpan){0},
            "Invalid nominal shape query or receiver", NULL, NULL);
    }
    StaticConstraintOutcome solved;
    if (!static_constraint_solve(solver, &solved)) { return false; }
    const StaticTerm *root = static_constraint_root(solver, value);
    if (root->kind == STATIC_TERM_VARIABLE) {
        *outcome = STATIC_CONSTRAINT_PENDING;
        return true;
    }
    *output = root;
    *outcome = STATIC_CONSTRAINT_COMPLETE;
    return true;
}
