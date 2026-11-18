/** @file
 * @brief Intern source-scoped application results independently of producer-input inference variables.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) keeps pending application equations finite.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) preserves lexical owner identity.
 */
#include "../internal.h"

/** Check source membership before dereferencing untrusted syntax identities;
 * internal variables are equations, never additional user/owner parameters.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) preserves immutable definitions.
 */
bool static_term_expression_result(StaticTermStore *store, const StaticTermEnvironment *environment,
    const AstTypeExpression *expression, const StaticTerm **output) {
    if (store->status != STATIC_TERM_OK) { return false; }
    if (output == NULL || expression == NULL || !static_term_has_environment(store, environment)) {
        return static_term_invalid(store, (SourceSpan){0}, "Invalid static expression result identity");
    }
    for (const StaticOccurrence *item = environment->definition->first_occurrence; item; item = item->next) {
        if (item->expression == expression) {
            StaticVariableIdentity identity = {.role = STATIC_VARIABLE_APPLICATION,
                .environment = environment, .producer = expression};
            return static_term_variable(store, identity, STATIC_CATEGORY_DEFERRED, NULL, expression->span, output);
        }
    }
    return static_term_invalid(store, (SourceSpan){0}, "Static expression does not belong to its supplied owner");
}
