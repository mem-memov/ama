/** @file
 * @brief Register ordinary Type forms with exact selected-export scalar identities.
 * [TYPE-01](../../../../../spec/language/declarations.md#type-01) requires one exact public selector.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) retains complete producer-input obligations.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) separates actual contexts and source facts.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns partial obligations.
 * [TPL-15](../../../../../spec/language/templates.md#tpl-15) requires every
 * supplied producer argument to participate in the shared checked source
 * evaluation, including operands with their own selected results.
 */
#include <stdint.h>
#include "internal.h"

/** Validate original root syntax in an ordinary declaration or namespace before
 * dereferencing expression data; static graph definitions are another domain.
 * [TYPE-01](../../../../../spec/language/declarations.md#type-01) fixes ordinary forms.
 */
static const StaticOccurrence *occurrence(const StaticTermEnvironment *environment, const AstTypeExpression *expression) {
    const StaticModule *module = environment->definition;
    if (module->definition->syntax.sections.kind == MODULE_STATIC) { return NULL; }
    for (const StaticOccurrence *site = module->first_occurrence; site; site = site->next) {
        if (site->receiver == NULL && site->root && site->expression == expression) { return site; }
    }
    return NULL;
}

/** A static path selects exactly one public result; its free input names are not
 * exports. An ordinary module cannot reinterpret a selector as an application.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) distinguishes supplier and value.
 */
static bool selector(StaticPublicationPlan *plan, const StaticOccurrence *site, StaticOrdinaryType *record) {
    const AstTypeExpression *expression = site->expression;
    bool selected = expression->selected_export.source != NULL;
    bool producer = site->origin && site->origin->syntax.sections.kind == MODULE_STATIC;
    if (!producer && !selected) { return true; }
    if (!producer || !selected) {
        return static_publication_fail(plan, STATIC_PUBLICATION_CONFLICT, expression->span,
            "A static module requires one named export selector; ordinary Types cannot have one");
    }
    record->producer = static_module_find(plan->results->guards->applications->terms->inventory, site->origin);
    record->selected = static_symbol_find(record->producer, expression->selected_export);
    if (!record->selected || (record->selected->role != STATIC_RESULT && record->selected->role != STATIC_ALIAS)) {
        return static_publication_fail(plan, STATIC_PUBLICATION_CONFLICT, expression->selected_export,
            "The static module has no public export with this exact name");
    }
    return true;
}

/** Preparing a local Type variable does not itself choose a foreign output:
 * the exact selected definition contributes its own structural equation later.
 * [TYPE-01](../../../../../spec/language/declarations.md#type-01) requires exact Type expressions and uniquely determined producer inputs.
 */
static bool prepare(StaticPublicationPlan *plan, const StaticResultContext *context,
    const StaticTermEnvironment *environment, const AstTypeExpression *expression, StaticOrdinaryType **output) {
    if (!static_publication_dependencies(plan)) { return false; }
    if (!output || !static_term_has_environment(plan->results->guards->applications->terms, environment)) {
        return static_publication_fail(plan, STATIC_PUBLICATION_INVALID, (SourceSpan){0}, "Invalid ordinary Type context or receiver");
    }
    const StaticOccurrence *site = occurrence(environment, expression);
    if (!site) {
        return static_publication_fail(plan, STATIC_PUBLICATION_INVALID, (SourceSpan){0}, "Ordinary Type syntax is not an original root occurrence");
    }
    for (StaticOrdinaryType *record = plan->first_type; record; record = record->next) {
        if (record->context == context && record->environment == environment && record->expression == expression) { *output = record; return true; }
    }
    if (plan->type_count == SIZE_MAX) {
        return static_publication_fail(plan, STATIC_PUBLICATION_OUT_OF_MEMORY, expression->span,
            "Ordinary Type record count exceeds representable storage");
    }
    StaticOrdinaryType prepared = {.context = context, .environment = environment, .expression = expression};
    if (!selector(plan, site, &prepared)) { return false; }
    StaticConstraintSolver *solver = plan->results->guards->applications->solver;
    if (context) {
        if (!static_result_use(plan->results, context, expression, NULL, &prepared.source)) {
            (void)static_publication_dependencies(plan); return false;
        }
        prepared.annotation = prepared.source->source;
    } else if (!static_result_source(plan->results, environment, expression, &prepared.annotation)) {
        (void)static_publication_dependencies(plan); return false;
    }
    prepared.value = prepared.annotation->value;
    SourceSpan requirement = prepared.producer ? expression->selected_export : expression->span;
    if (!static_constraint_describe(solver, prepared.value, STATIC_CATEGORY_TYPE, NULL, expression->span) ||
        !static_constraint_require(solver, prepared.value, requirement)) {
        (void)static_publication_dependencies(plan); return false;
    }
    StaticOrdinaryType *record = static_publication_allocate(plan, sizeof(*record));
    if (!record) { return false; }
    *record = prepared;
    record->next = plan->first_type;
    plan->first_type = record;
    plan->type_count++;
    *output = record;
    return true;
}

/** Actual incoming identity is checked before accessing its environment. Metadata
 * registration alone cannot create this provenance or authorize body expansion.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) retains guarded actual uses.
 */
bool static_ordinary_type(StaticPublicationPlan *plan, const StaticResultContext *context,
    const AstTypeExpression *expression, StaticOrdinaryType **output) {
    if (!static_publication_dependencies(plan)) { return false; }
    if (!static_result_has_context(plan->results, context)) {
        return static_publication_fail(plan, STATIC_PUBLICATION_INVALID, (SourceSpan){0}, "Invalid actual ordinary Type context");
    }
    return prepare(plan, context, context->creation->owner->environment, expression, output);
}

/** Exact nominal/generic source environments expose only original finite Type
 * annotations, with no manufactured incoming context or readiness certificate.
 * [TYPE-01](../../../../../spec/language/declarations.md#type-01) preserves source Type identity.
 */
bool static_ordinary_source(StaticPublicationPlan *plan, const StaticTermEnvironment *environment,
    const AstTypeExpression *expression, StaticOrdinaryType **output) {
    return prepare(plan, NULL, environment, expression, output);
}
