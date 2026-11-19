/** @file
 * @brief Equate exact named static output selections with their original local receiver symbols.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves equality without definition reads.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) uses original receiver binding identity.
 */
#include "internal.h"

/** Accepted source occurrence identity fixes the first alias versus later
 * equality classification before any semantic work is scheduled.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) forbids spelling-based capture.
 */
static const StaticSymbol *receiver(const StaticResultUse *use, const AstStaticBinding *binding) {
    for (const StaticOccurrence *site = use->source->environment->definition->first_occurrence; site; site = site->next) {
        if (site->expression == use->source->expression && site->receiver == binding) { return site->symbol; }
    }
    return NULL;
}

/** Every request is named and complete. A receiver equality never reads or
 * redefines that receiver; selected value-definition reads are separate.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) admits anchored equality cycles.
 */
bool static_result_receivers(StaticResultPlan *plan, StaticResultUse *use) {
    if (!use->source->producer || !use->producer) { return true; }
    StaticTermStore *terms = plan->guards->applications->terms;
    for (const BindingRequest *request = use->requests->first; request; request = request->next) {
        if (request->direction != BINDING_STATIC_OUTPUT) { continue; }
        const StaticSymbol *target = receiver(use, request->syntax.static_binding);
        if (!request->slot || !target) {
            return static_result_fail(plan, STATIC_RESULT_INVALID, request->span, "Static output has no actual slot or original receiver");
        }
        const StaticTerm *selected, *destination;
        if (!static_term_symbol(terms, use->producer->creation->owner->environment, request->slot->static_symbol, &selected) ||
            !static_term_symbol(terms, use->source->environment, target, &destination) ||
            !static_constraint_equate(plan->guards->applications->solver, selected, destination, request->span)) {
            (void)static_result_dependencies(plan); return false;
        }
    }
    return true;
}
