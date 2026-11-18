/** @file
 * @brief Equate static output receivers with exact actual producer result symbols.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) separates receiver equality from definition reads.
 */
#include "internal.h"

/** Receiver occurrences were accepted by the lexical inventory. Pointer
 * identity selects the original binding, including a newly introduced alias.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) preserves scope.
 */
static const StaticSymbol *receiver(const StaticResultUse *use, const AstStaticBinding *binding) {
    for (const StaticOccurrence *site = use->source->environment->definition->first_occurrence; site; site = site->next) {
        if (site->expression == use->source->expression && site->receiver == binding) { return site->symbol; }
    }
    return NULL;
}

/** Actual application requests already validated names, positions and omissions.
 * Skips add no equation; receiver use alone does not read its value definition.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves equality constraints.
 */
bool static_result_receivers(StaticResultPlan *plan, StaticResultUse *use) {
    const BindingApplication *requests = use->application->context.requests;
    if (requests == NULL || requests->output_count == 0) { return true; }
    if (use->producer == NULL) { return true; }
    StaticTermStore *terms = plan->guards->applications->terms;
    for (const BindingRequest *request = requests->first; request; request = request->next) {
        if (request->direction != BINDING_STATIC_OUTPUT || request->omitted) { continue; }
        const StaticSymbol *target = receiver(use, request->syntax.static_binding);
        if (request->slot == NULL || target == NULL) {
            return static_result_fail(plan, STATIC_RESULT_INVALID, request->span, "Static output has no accepted actual slot or receiver");
        }
        const StaticTerm *selected, *destination;
        if (!static_term_symbol(terms, use->producer->creation->owner->environment,
                request->slot->static_symbol, &selected) ||
            !static_term_symbol(terms, use->source->environment, target, &destination) ||
            !static_constraint_equate(plan->guards->applications->solver, selected, destination, request->span)) {
            (void)static_result_dependencies(plan); return false;
        }
    }
    return true;
}
