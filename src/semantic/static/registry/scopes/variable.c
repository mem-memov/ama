/** @file
 * @brief Associate original local static variables with their closed specialization scope.
 * [TPL-05](../../../../../spec/language/templates.md#tpl-05) shares compile-time metadata only.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) retains every lexical identity component.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) checks incoming provenance before reuse.
 */
#include "../internal.h"
#include "../../terms/internal.h"

/** Copy the entire variable identity, replacing only its proven-equal owner.
 * Applied formals read captures instead of inventing unused canonical holes.
 * An equation connects original and canonical values; it never rewrites a path.
 * [TPL-05](../../../../../spec/language/templates.md#tpl-05) preserves full-key sharing.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) keeps distinct source operations distinct.
 */
bool static_registry_scope_variable(StaticRegistryPlan *plan, StaticRegistryUse *use,
    const StaticTerm *variable, const StaticTerm **output, StaticRegistryKeyState *state) {
    if (!static_registry_dependencies(plan)) { return false; }
    StaticTermStore *terms = plan->guards->applications->terms;
    if (!output || !state || !static_registry_has_use(plan, use) || !static_term_has_value(terms, variable) ||
        variable->kind != STATIC_TERM_VARIABLE ||
        variable->as.variable.environment != use->path->owner->environment) {
        return static_registry_fail(plan, STATIC_REGISTRY_INVALID, (SourceSpan){0},
            "Invalid original variable or specialization use scope");
    }
    StaticRegistryUse *refreshed;
    StaticRegistryKeyState key_state;
    if (!static_registry_use(plan, use->path, &refreshed, &key_state)) { return false; }
    if (key_state == STATIC_REGISTRY_KEY_PENDING) { *state = key_state; return true; }
    StaticVariableIdentity identity = variable->as.variable;
    identity.environment = use->candidate->key;
    const StaticTerm *canonical;
    bool prepared = identity.role == STATIC_VARIABLE_FORMAL ?
        static_term_symbol(terms, identity.environment, identity.symbol, &canonical) :
        static_term_variable(terms, identity, variable->category, variable->signature, variable->site, &canonical);
    if (!prepared || !static_constraint_equate(plan->guards->applications->solver,
        variable, canonical, variable->site)) {
        (void)static_registry_dependencies(plan);
        return false;
    }
    *output = canonical;
    *state = key_state;
    return true;
}
