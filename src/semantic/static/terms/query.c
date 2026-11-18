/** @file
 * @brief Query canonical value equality and cached free-variable closedness.
 * [TPL-05](../../../../spec/language/templates.md#tpl-05) defines nominal equality.
 * [TPL-10](../../../../spec/language/templates.md#tpl-10) keeps solving separate.
 * [TPL-08](../../../../spec/language/templates.md#tpl-08) treats residual formals as local binders.
 */
#include "internal.h"

/** All children precede parents and all vertices are interned; pointer equality
 * is therefore exact structural equality within this owner, not a solver result.
 * [TPL-05](../../../../spec/language/templates.md#tpl-05) fixes value identity.
 */
bool static_term_equal(StaticTermStore *store, const StaticTerm *left, const StaticTerm *right, bool *output) {
    if (store->status != STATIC_TERM_OK) { return false; }
    if (output == NULL || !static_term_has_value(store, left) || !static_term_has_value(store, right)) {
        return static_term_invalid(store, (SourceSpan){0}, "Invalid static equality handle");
    }
    *output = left == right;
    return true;
}

/** Variables are free; nominal captures conjoin child closedness and residuals
 * are local binders. [TPL-08](../../../../spec/language/templates.md#tpl-08).
 */
bool static_term_ground(StaticTermStore *store, const StaticTerm *term, bool *output) {
    if (store->status != STATIC_TERM_OK) { return false; }
    if (output == NULL || !static_term_has_value(store, term)) {
        return static_term_invalid(store, (SourceSpan){0}, "Invalid static closedness handle");
    }
    *output = term->ground;
    return true;
}
