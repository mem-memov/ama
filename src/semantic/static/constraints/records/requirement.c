/** @file
 * @brief Retain explicit required-closure obligations without inventing type information.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) distinguishes unresolved required values.
 */
#include <stdint.h>
#include "../internal.h"

/** Requirements are identity predicates; repeated uses need only one closure
 * check, while the original site remains diagnostic evidence.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) requires unique closure.
 */
bool static_constraint_require(StaticConstraintSolver *solver, const StaticTerm *term, SourceSpan site) {
    if (solver->status != STATIC_CONSTRAINT_OK) { return false; }
    if (!static_term_has_value(solver->terms, term)) {
        return static_constraint_failure(solver, STATIC_CONSTRAINT_INVALID, site,
            "Required static value must belong to its term owner", NULL, NULL);
    }
    for (const StaticConstraintRequirement *item = solver->first_requirement; item; item = item->next) {
        if (item->term == term) { return true; }
    }
    if (solver->requirement_count == SIZE_MAX) {
        return static_constraint_failure(solver, STATIC_CONSTRAINT_OUT_OF_MEMORY, site,
            "Static requirement count exceeds representable storage", term, NULL);
    }
    StaticConstraintRequirement *item = static_constraint_allocate(solver, sizeof(*item));
    if (item == NULL) { return false; }
    *item = (StaticConstraintRequirement){.term = term, .site = site};
    if (solver->last_requirement != NULL) { solver->last_requirement->next = item; }
    else { solver->first_requirement = item; }
    solver->last_requirement = item;
    solver->requirement_count++;
    solver->dirty = true;
    return true;
}
