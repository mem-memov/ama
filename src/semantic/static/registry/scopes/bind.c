/** @file
 * @brief Bind one validated variable to its closed canonical owner without changing lexical identity.
 * [TPL-05](../../../../../spec/language/templates.md#tpl-05) shares compile-time equations only.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) preserves every original identity component.
 */
#include "../internal.h"
#include "../../terms/internal.h"

/** Copy the whole identity rather than enumerating roles/occurrence fields.
 * Formals read actual captures; other variables keep their complete source key.
 * Identical handles need no tautological equation or apparent solver progress.
 * [TPL-05](../../../../../spec/language/templates.md#tpl-05) keeps equal-key analysis consistent.
 */
bool static_registry_bind_scope(StaticRegistryPlan *plan, const StaticRegistryUse *use,
    const StaticTerm *variable, const StaticTerm **output) {
    StaticTermStore *terms = plan->guards->applications->terms;
    StaticVariableIdentity identity = variable->as.variable;
    identity.environment = use->candidate->key;
    const StaticTerm *canonical;
    bool prepared = identity.role == STATIC_VARIABLE_FORMAL ?
        static_term_symbol(terms, identity.environment, identity.symbol, &canonical) :
        static_term_variable(terms, identity, variable->category, variable->signature, variable->site, &canonical);
    if (!prepared || (variable != canonical && !static_constraint_equate(plan->guards->applications->solver,
        variable, canonical, variable->site))) {
        (void)static_registry_dependencies(plan); return false;
    }
    *output = canonical;
    return true;
}
