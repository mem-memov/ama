/** @file
 * @brief Read exact independently refined descriptions without mutating source metadata.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) separates solved evidence from syntax.
 */
#include "../internal.h"

/** Metadata facts override deferred descriptions only after the refinement
 * operation has preserved known declarations. No alias origin is invented.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) requires monotonicity.
 */
void static_constraint_description(const StaticConstraintSolver *solver, const StaticTerm *term,
    StaticCategory *category, const StaticSignature **signature) {
    for (const StaticConstraintMetadata *fact = solver->first_metadata; fact; fact = fact->next) {
        if (fact->term == term) { *category = fact->category; *signature = fact->signature; return; }
    }
    *category = term->category;
    *signature = term->signature;
}
