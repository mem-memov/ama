/** @file
 * @brief Read known contracts connected by explicit equations without searching for a constructor.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) preserves all alias-class restrictions.
 */
#include "../internal.h"

/** A solved nominal is authoritative. An unknown representative may use an
 * explicit same-class declaration's contract; all known class contracts were
 * checked together by T176, so this cannot choose an origin or source order.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) propagates explicit equalities.
 */
void static_application_description(const StaticApplicationPlan *plan, const StaticTerm *value,
    StaticCategory *category, const StaticSignature **signature) {
    const StaticTerm *root = static_constraint_root(plan->solver, value);
    static_constraint_description(plan->solver, root, category, signature);
    if (*category != STATIC_CATEGORY_DEFERRED) { return; }
    static_constraint_description(plan->solver, value, category, signature);
    if (*category != STATIC_CATEGORY_DEFERRED) { return; }
    for (const StaticConstraintVariable *item = plan->solver->first_variable; item; item = item->next) {
        if (static_constraint_root(plan->solver, item->variable) != root) { continue; }
        static_constraint_description(plan->solver, item->variable, category, signature);
        if (*category != STATIC_CATEGORY_DEFERRED) { return; }
    }
}
