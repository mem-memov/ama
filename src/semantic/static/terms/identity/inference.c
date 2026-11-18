/** @file
 * @brief Key inference variables by original formal and registered lexical occurrence.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) separates field occurrences.
 */
#include "../internal.h"

/** Search accepted source occurrences before dereferencing caller-provided syntax.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) requires lexical ownership.
 */
static bool registered_site(const StaticModule *module, const AstNode *declaration,
    const AstTypeExpression *producer) {
    for (const StaticOccurrence *item = module->first_occurrence; item != NULL; item = item->next) {
        if (declaration != NULL && item->statement == declaration && declaration->kind == AST_DECLARATION) {
            return true;
        }
        if (producer != NULL && item->expression == producer) { return true; }
    }
    return false;
}

/** Residual formals remain declarations; each requested hole receives a separate
 * role and occurrence key. [TPL-08](../../../../../spec/language/templates.md#tpl-08).
 */
bool static_term_inference(StaticTermStore *store, const StaticTermEnvironment *environment,
    const StaticSymbol *formal, const AstNode *declaration, const AstTypeExpression *producer,
    const StaticTerm **output) {
    if (store->status != STATIC_TERM_OK) { return false; }
    if (output == NULL || !static_term_has_environment(store, environment) ||
        !static_term_has_symbol(store, formal) || formal->role != STATIC_PARAMETER ||
        ((declaration == NULL) == (producer == NULL)) ||
        !registered_site(environment->definition, declaration, producer)) {
        return static_term_invalid(store, (SourceSpan){0}, "Invalid static inference occurrence");
    }
    StaticVariableIdentity identity = {.environment = environment, .symbol = formal,
        .declaration = declaration, .producer = producer,
        .role = declaration != NULL ? STATIC_VARIABLE_FIELD : STATIC_VARIABLE_PRODUCER};
    SourceSpan site = declaration != NULL ? declaration->span : producer->span;
    return static_term_variable(store, identity, formal->category, formal->signature, site, output);
}
