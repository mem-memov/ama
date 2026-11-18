/** @file
 * @brief Recheck every retained alias-class category and recursive signature restriction.
 * [TPL-07](../../../../../spec/language/templates.md#tpl-07) matches named signatures.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) preserves earlier constraints.
 */
#include "../internal.h"
#include "../../signature/signature.h"

/** Unknown categories impose no invented declaration; deferred signature entries
 * stay explicit obligations and cannot hide any known disagreement.
 * [TPL-07](../../../../../spec/language/templates.md#tpl-07) fixes complete signatures.
 */
bool static_constraint_metadata_match(StaticConstraintSolver *solver, const StaticTerm *left,
    const StaticTerm *right, SourceSpan site, bool *pending) {
    StaticCategory left_category, right_category;
    const StaticSignature *left_signature, *right_signature;
    static_constraint_description(solver, left, &left_category, &left_signature);
    static_constraint_description(solver, right, &right_category, &right_signature);
    if (left_category == STATIC_CATEGORY_DEFERRED || right_category == STATIC_CATEGORY_DEFERRED) { return true; }
    if (left_category != right_category) {
        return static_constraint_failure(solver, STATIC_CONSTRAINT_CONFLICT, site,
            "Static equality has incompatible Type and Template categories", left, right);
    }
    if (left_category == STATIC_CATEGORY_TEMPLATE) {
        StaticSignatureComparison result;
        if (!static_constraint_signature(solver, left_signature, right_signature, site, &result)) { return false; }
        if (result == STATIC_SIGNATURE_DIFFERENT) {
            return static_constraint_failure(solver, STATIC_CONSTRAINT_CONFLICT, site,
                "Static equality has incompatible constructor signatures", left, right);
        }
        if (result == STATIC_SIGNATURE_PENDING) {
            *pending = true;
            if (solver->pending_left == NULL) {
                solver->pending_left = left;
                solver->pending_right = right;
                solver->pending_site = site;
            }
        }
    }
    return true;
}

/** Choose an actual equation belonging to this class, retaining original source
 * parent links for later chain rendering. [TPL-10](../../../../../spec/language/templates.md#tpl-10).
 */
static SourceSpan class_site(const StaticConstraintSolver *solver, const StaticTerm *root) {
    for (const StaticConstraintEquation *item = solver->first_equation; item; item = item->next) {
        if (static_constraint_root(solver, item->left) == root ||
            static_constraint_root(solver, item->right) == root) { return item->site; }
    }
    return root->site;
}

/** Pairwise class checks preserve known restrictions even when the canonical
 * representative originally had deferred metadata; no first-actual precedence.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) requires joint consistency.
 */
bool static_constraint_classes(StaticConstraintSolver *solver, bool *pending) {
    for (StaticConstraintVariable *item = solver->first_variable; item; item = item->next) {
        item->representative = static_constraint_root(solver, item->variable);
    }
    for (const StaticConstraintVariable *left = solver->first_variable; left; left = left->next) {
        const StaticTerm *root = left->representative;
        bool no_witness = solver->pending_left == NULL;
        if (!static_constraint_metadata_match(solver, left->variable, root, left->variable->site, pending)) {
            if (solver->status == STATIC_CONSTRAINT_CONFLICT) { solver->error.primary.span = class_site(solver, root); }
            return false;
        }
        if (no_witness && solver->pending_left != NULL) { solver->pending_site = class_site(solver, root); }
        for (const StaticConstraintVariable *right = left->next; right; right = right->next) {
            if (right->representative != root) { continue; }
            no_witness = solver->pending_left == NULL;
            if (!static_constraint_metadata_match(solver, left->variable, right->variable, left->variable->site, pending)) {
                if (solver->status == STATIC_CONSTRAINT_CONFLICT) { solver->error.primary.span = class_site(solver, root); }
                return false;
            }
            if (no_witness && solver->pending_left != NULL) { solver->pending_site = class_site(solver, root); }
        }
    }
    return true;
}
