/** @file
 * @brief Register ordinary Type forms while keeping selected static exports disconnected from consumer inference.
 * [TYPE-01](../../../../../spec/language/declarations.md#type-01) requires one exact public selector.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) prevents ordinary factory binding blocks.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns partial obligations.
 */
#include <stdint.h>
#include "internal.h"

/** Validate original root syntax in an ordinary declaration or namespace before
 * dereferencing expression data; static graph definitions are another domain.
 * [TYPE-01](../../../../../spec/language/declarations.md#type-01) fixes ordinary forms.
 */
static const StaticOccurrence *occurrence(const StaticResultContext *context, const AstTypeExpression *expression) {
    const StaticModule *module = context->creation->owner->environment->definition;
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

/** Preparing a local Type variable does not constrain the selected foreign
 * output: only the independent export operation can add that equality later.
 * [TYPE-01](../../../../../spec/language/declarations.md#type-01) forbids receiver-driven factory inference.
 */
bool static_ordinary_type(StaticPublicationPlan *plan, const StaticResultContext *context,
    const AstTypeExpression *expression, StaticOrdinaryType **output) {
    if (!static_publication_dependencies(plan)) { return false; }
    if (!output || !static_result_has_context(plan->results, context)) {
        return static_publication_fail(plan, STATIC_PUBLICATION_INVALID, (SourceSpan){0}, "Invalid ordinary Type context or receiver");
    }
    const StaticOccurrence *site = occurrence(context, expression);
    if (!site) {
        return static_publication_fail(plan, STATIC_PUBLICATION_INVALID, (SourceSpan){0}, "Ordinary Type syntax is not an original root occurrence");
    }
    for (StaticOrdinaryType *record = plan->first_type; record; record = record->next) {
        if (record->context == context && record->expression == expression) { *output = record; return true; }
    }
    if (plan->type_count == SIZE_MAX) {
        return static_publication_fail(plan, STATIC_PUBLICATION_OUT_OF_MEMORY, expression->span,
            "Ordinary Type record count exceeds representable storage");
    }
    StaticOrdinaryType prepared = {.context = context, .expression = expression};
    if (!selector(plan, site, &prepared)) { return false; }
    const StaticTermEnvironment *environment = context->creation->owner->environment;
    StaticConstraintSolver *solver = plan->results->guards->applications->solver;
    if (prepared.producer) {
        if (!static_term_expression_result(solver->terms, environment, expression, &prepared.value)) {
            (void)static_publication_dependencies(plan); return false;
        }
    } else {
        if (!static_result_use(plan->results, context, expression, NULL, &prepared.source)) {
            (void)static_publication_dependencies(plan); return false;
        }
        prepared.value = prepared.source->source->value;
    }
    if (!static_constraint_describe(solver, prepared.value, STATIC_CATEGORY_TYPE, NULL, expression->span) ||
        !static_constraint_require(solver, prepared.value, expression->span)) {
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
