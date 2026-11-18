/** @file
 * @brief Refine one equality or decompose exact original nominal capture keys.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) compares full nominal structure.
 */
#include "../internal.h"

/** Equal origins still require the same fixed/residual original formal sets;
 * corresponding values produce equations rather than being replaced or guessed.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) preserves complete bindings.
 */
static bool decompose(StaticConstraintSolver *solver, const StaticTerm *left,
    const StaticTerm *right, const StaticConstraintEquation *equation, bool *changed) {
    const StaticNominalTerm *a = &left->as.nominal, *b = &right->as.nominal;
    if (a->origin != b->origin || a->capture_count != b->capture_count || a->residual_count != b->residual_count) {
        return static_constraint_failure(solver, STATIC_CONSTRAINT_CONFLICT, equation->site,
            "Static equality has different nominal origins or binding sets", left, right);
    }
    const StaticTermCapture *other = b->first_capture;
    for (const StaticTermCapture *capture = a->first_capture; capture; capture = capture->next, other = other->next) {
        if (capture->formal != other->formal) {
            return static_constraint_failure(solver, STATIC_CONSTRAINT_CONFLICT, equation->site,
                "Static equality fixes different original constructor formals", left, right);
        }
        bool added;
        if (!static_constraint_pair(solver, capture->value, other->value, equation->site, equation, &added)) { return false; }
        *changed = *changed || added;
    }
    return true;
}

/** Activate original variables before dereferencing so their category/signature
 * obligations survive union with a differently declared representative.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) preserves earlier equations.
 */
bool static_constraint_process(StaticConstraintSolver *solver,
    const StaticConstraintEquation *equation, bool *changed, bool *pending) {
    const StaticTerm *original[] = {equation->left, equation->right};
    for (size_t index = 0; index < 2; index++) {
        if (original[index]->kind == STATIC_TERM_VARIABLE && static_constraint_variable(solver, original[index]) == NULL) {
            return false;
        }
    }
    if (!static_constraint_metadata_match(solver, equation->left, equation->right, equation->site, pending)) { return false; }
    const StaticTerm *left = static_constraint_root(solver, equation->left);
    const StaticTerm *right = static_constraint_root(solver, equation->right);
    if (!static_constraint_metadata_match(solver, left, right, equation->site, pending)) { return false; }
    if (left == right) { return true; }
    if (left->kind == STATIC_TERM_VARIABLE) { return static_constraint_bind(solver, left, right, equation, changed); }
    if (right->kind == STATIC_TERM_VARIABLE) { return static_constraint_bind(solver, right, left, equation, changed); }
    return decompose(solver, left, right, equation, changed);
}
