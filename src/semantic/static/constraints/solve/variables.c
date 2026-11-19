/** @file
 * @brief Follow and refine variable links with exact representative and occurs invariants.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) rejects infinite substitutions.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns partial state.
 */
#include <stdint.h>
#include "../internal.h"

/** Exact immutable keys keep source variable identity independent of its solution.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) preserves scoped terms.
 */
static StaticConstraintVariable *find(const StaticConstraintSolver *solver, const StaticTerm *term) {
    return term->ordinal < solver->variable_capacity ? solver->variables[term->ordinal] : NULL;
}

/** Each variable-only link decreases original ordinal, and nominal links cannot
 * lead back through captures after occurs checking, so following links terminates.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) excludes substitution cycles.
 */
const StaticTerm *static_constraint_root(const StaticConstraintSolver *solver, const StaticTerm *term) {
    while (term->kind == STATIC_TERM_VARIABLE) {
        const StaticConstraintVariable *item = find(solver, term);
        if (item == NULL || item->value == NULL) { break; }
        term = item->value;
    }
    return term;
}

/** Register once so future aliases cannot discard a declaration's restrictions.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) retains prior equations.
 */
StaticConstraintVariable *static_constraint_variable(StaticConstraintSolver *solver, const StaticTerm *term) {
    StaticConstraintVariable *item = find(solver, term);
    if (item != NULL) { return item; }
    if (solver->variable_count == SIZE_MAX) {
        (void)static_constraint_failure(solver, STATIC_CONSTRAINT_OUT_OF_MEMORY, term->site,
            "Static variable count exceeds representable storage", term, NULL);
        return NULL;
    }
    if (!static_constraint_lookup_capacity(solver, term->ordinal + 1)) { return NULL; }
    item = static_constraint_allocate(solver, sizeof(*item));
    if (item == NULL) { return NULL; }
    item->variable = term;
    item->next = solver->first_variable;
    solver->first_variable = item;
    solver->variable_count++;
    solver->variables[term->ordinal] = item;
    return item;
}

/** Both operands are representatives; an existing solved record is never
 * overwritten. [TPL-10](../../../../../spec/language/templates.md#tpl-10).
 */
bool static_constraint_bind(StaticConstraintSolver *solver, const StaticTerm *variable,
    const StaticTerm *value, const StaticConstraintEquation *equation, bool *changed) {
    if (value->kind == STATIC_TERM_VARIABLE && value->ordinal > variable->ordinal) {
        const StaticTerm *swap = variable; variable = value; value = swap;
    }
    StaticConstraintVariable *record = static_constraint_variable(solver, variable);
    if (record == NULL) { return false; }
    if (value->kind == STATIC_TERM_VARIABLE) {
        if (static_constraint_variable(solver, value) == NULL) { return false; }
    } else {
        bool occurs;
        if (!static_constraint_contains(solver, value, variable, &occurs)) { return false; }
        if (occurs) {
            return static_constraint_failure(solver, STATIC_CONSTRAINT_CONFLICT, equation->site,
                "Static equality requires an infinite substitution term", variable, value);
        }
    }
    record->value = value;
    record->reason = equation;
    solver->dirty = true;
    *changed = true;
    return true;
}
