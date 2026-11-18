/** @file
 * @brief Deduplicate finite unordered equality pairs before publishing source-backed work.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) shares structural constraints.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns partial records.
 */
#include <stdint.h>
#include "../internal.h"

/** Canonical pair orientation is based on immutable ordinals, not source names
 * or traversal order. [TPL-10](../../../../../spec/language/templates.md#tpl-10).
 */
bool static_constraint_pair(StaticConstraintSolver *solver, const StaticTerm *left, const StaticTerm *right,
    SourceSpan site, const StaticConstraintEquation *parent, bool *added) {
    if (right->ordinal < left->ordinal) { const StaticTerm *swap = left; left = right; right = swap; }
    for (const StaticConstraintEquation *item = solver->first_equation; item; item = item->next) {
        if (item->left == left && item->right == right) { *added = false; return true; }
    }
    if (solver->equation_count == SIZE_MAX) {
        return static_constraint_failure(solver, STATIC_CONSTRAINT_OUT_OF_MEMORY, site,
            "Static equation count exceeds representable storage", left, right);
    }
    StaticConstraintEquation *item = static_constraint_allocate(solver, sizeof(*item));
    if (item == NULL) { return false; }
    *item = (StaticConstraintEquation){.left = left, .right = right, .site = site, .parent = parent};
    if (solver->last_equation != NULL) { solver->last_equation->next = item; }
    else { solver->first_equation = item; }
    solver->last_equation = item;
    solver->equation_count++;
    solver->dirty = true;
    *added = true;
    return true;
}

/** Only exact published term handles can become equation operands; consistency
 * is checked by the separate solve step, not record construction.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) separates evidence from solutions.
 */
bool static_constraint_equate(StaticConstraintSolver *solver, const StaticTerm *left,
    const StaticTerm *right, SourceSpan site) {
    if (solver->status != STATIC_CONSTRAINT_OK) { return false; }
    if (!static_term_has_value(solver->terms, left) || !static_term_has_value(solver->terms, right)) {
        return static_constraint_failure(solver, STATIC_CONSTRAINT_INVALID, site,
            "Static equation operands must belong to its term owner", NULL, NULL);
    }
    bool added;
    return static_constraint_pair(solver, left, right, site, NULL, &added);
}
