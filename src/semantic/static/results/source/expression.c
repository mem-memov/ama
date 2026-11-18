/** @file
 * @brief Bind exact original expression heads in their lexical owner without interpreting names as text paths.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) preserves structural scope.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) separates producer/result variables.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns partial records.
 */
#include <stdint.h>
#include "../internal.h"

/** Receiver occurrences never stand for expression heads. Exact registered
 * identity precedes every dereference of a caller's syntax pointer.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) preserves source identity.
 */
static const StaticOccurrence *occurrence(const StaticTermEnvironment *environment, const AstTypeExpression *expression) {
    for (const StaticOccurrence *site = environment->definition->first_occurrence; site; site = site->next) {
        if (site->receiver == NULL && site->expression == expression) { return site; }
    }
    return NULL;
}

/** A path already has its accepted module origin; a placeholder already has
 * its exact scoped symbol. Neither branch performs a filesystem/name search.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) forbids name capture.
 */
static bool head(StaticResultPlan *plan, const StaticTermEnvironment *environment,
    const StaticOccurrence *site, const StaticTerm **output) {
    StaticTermStore *terms = plan->guards->applications->terms;
    if (site->expression->kind == AST_TYPE_PLACEHOLDER && site->symbol != NULL) {
        if (static_term_symbol(terms, environment, site->symbol, output)) { return true; }
    } else if (site->expression->kind == AST_TYPE_MODULE_PATH && site->origin != NULL) {
        const StaticModule *module = static_module_find(terms->inventory, site->origin);
        if (module != NULL && static_term_nominal(terms, module, NULL, 0, site->expression->head, output)) { return true; }
    }
    if (!static_result_dependencies(plan)) { return false; }
    return static_result_fail(plan, STATIC_RESULT_INVALID, site->site, "Static expression has no accepted bound head identity");
}

/** Preparation preserves an independent expression result key without equating
 * it with a selected output or pretending that a pending application is closed.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves producer denotation.
 */
bool static_result_source(StaticResultPlan *plan, const StaticTermEnvironment *environment,
    const AstTypeExpression *expression, const StaticResultSource **output) {
    if (!static_result_dependencies(plan)) { return false; }
    StaticTermStore *terms = plan->guards->applications->terms;
    if (output == NULL || !static_term_has_environment(terms, environment)) {
        return static_result_fail(plan, STATIC_RESULT_INVALID, (SourceSpan){0}, "Invalid static expression environment or receiver");
    }
    const StaticOccurrence *site = occurrence(environment, expression);
    if (site == NULL) {
        return static_result_fail(plan, STATIC_RESULT_INVALID, (SourceSpan){0}, "Expression does not belong to its lexical environment");
    }
    for (const StaticResultSource *source = plan->first_source; source; source = source->next) {
        if (source->environment == environment && source->expression == expression) { *output = source; return true; }
    }
    if (plan->source_count == SIZE_MAX) {
        return static_result_fail(plan, STATIC_RESULT_OUT_OF_MEMORY, site->site,
            "Static expression source count exceeds representable storage");
    }
    const StaticTerm *head_value, *value;
    if (!head(plan, environment, site, &head_value)) { return false; }
    if (!static_term_expression_result(terms, environment, expression, &value)) {
        (void)static_result_dependencies(plan); return false;
    }
    StaticResultSource *source = static_result_allocate(plan, sizeof(*source));
    if (source == NULL) { return false; }
    *source = (StaticResultSource){.environment = environment, .expression = expression,
        .occurrence = site, .head = head_value, .value = value, .next = plan->first_source};
    plan->first_source = source;
    plan->source_count++;
    *output = source;
    return true;
}
