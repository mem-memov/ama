/** @file
 * @brief Bind original result and first receiver-alias definitions without conflating their expression values.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves immutable definition identity.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) preserves original environments.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns partial records.
 */
#include <stdint.h>
#include "../internal.h"

/** An alias's original receiver pointer identifies its defining producer
 * expression exactly; a later equality receiver cannot overwrite that definition.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) forbids alias reassignment.
 */
static const AstTypeExpression *expression(const StaticSymbol *symbol) {
    if (symbol->role == STATIC_RESULT) {
        if (symbol->declaration && symbol->declaration->kind == AST_STATIC_RESULT) {
            return symbol->declaration->as.static_result.expression;
        }
    } else if (symbol->role == STATIC_ALIAS && symbol->receiver != NULL) {
        for (const StaticOccurrence *site = symbol->owner->first_occurrence; site; site = site->next) {
            if (site->receiver == symbol->receiver && site->symbol == symbol) { return site->expression; }
        }
    }
    return NULL;
}

/** Derived definition terms remain scoped variables until guarded equations
 * supply their value. No free owner parameter is introduced by either role.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) separates results from formals.
 */
bool static_result_definition(StaticResultPlan *plan, const StaticTermEnvironment *environment,
    const StaticSymbol *symbol, const StaticResultDefinition **output) {
    if (!static_result_dependencies(plan)) { return false; }
    StaticTermStore *terms = plan->guards->applications->terms;
    if (output == NULL || !static_term_has_environment(terms, environment) || !static_term_has_symbol(terms, symbol) ||
        symbol->owner != environment->definition || (symbol->role != STATIC_RESULT && symbol->role != STATIC_ALIAS)) {
        return static_result_fail(plan, STATIC_RESULT_INVALID, (SourceSpan){0}, "Invalid static result or alias definition identity");
    }
    for (const StaticResultDefinition *definition = plan->first_definition; definition; definition = definition->next) {
        if (definition->environment == environment && definition->symbol == symbol) { *output = definition; return true; }
    }
    if (plan->definition_count == SIZE_MAX) {
        return static_result_fail(plan, STATIC_RESULT_OUT_OF_MEMORY, symbol->site,
            "Static result definition count exceeds representable storage");
    }
    const AstTypeExpression *syntax = expression(symbol);
    if (syntax == NULL) {
        return static_result_fail(plan, STATIC_RESULT_INVALID, symbol->site, "Static definition has no original defining expression");
    }
    const StaticResultSource *source;
    const StaticTerm *value;
    if (!static_result_source(plan, environment, syntax, &source)) { return false; }
    if (!static_term_symbol(terms, environment, symbol, &value)) {
        (void)static_result_dependencies(plan); return false;
    }
    StaticResultDefinition *definition = static_result_allocate(plan, sizeof(*definition));
    if (definition == NULL) { return false; }
    *definition = (StaticResultDefinition){.environment = environment, .symbol = symbol, .source = source,
        .receiver = symbol->role == STATIC_ALIAS ? symbol->receiver : NULL, .value = value,
        .next = plan->first_definition};
    plan->first_definition = definition;
    plan->definition_count++;
    *output = definition;
    return true;
}
