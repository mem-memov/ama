/** @file
 * @brief Supply independent original receiver definitions without inventing producer dependency edges.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) distinguishes equality and definition graphs.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) preserves lexical names.
 */
#include "internal.h"

/** Registered occurrences distinguish first alias definitions from later
 * same-spelling equality receivers without textual matching or source guessing.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) preserves original scope.
 */
const StaticSymbol *static_publication_receiver(const StaticResultUse *use, const AstStaticBinding *binding) {
    for (const StaticOccurrence *site = use->source->environment->definition->first_occurrence; site; site = site->next) {
        if (site->expression == use->source->expression && site->receiver == binding) { return site->symbol; }
    }
    return NULL;
}

/** A fresh alias supplies no additional information. Existing result/alias
 * definitions are independent roots; their actual RHS dependencies still enter
 * T178 through T179. Parameters retain their current external evidence.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) permits receiver inference.
 */
bool static_publication_evidence(StaticPublicationWork *work, const StaticResultUse *use) {
    StaticPublicationPlan *plan = work->plan;
    for (const AstStaticBinding *binding = use->source->expression->first; binding; binding = binding->next) {
        if (binding->kind != AST_STATIC_OUTPUT || binding->omitted) { continue; }
        const StaticSymbol *symbol = static_publication_receiver(use, binding);
        if (symbol == NULL) {
            return static_publication_fail(plan, STATIC_PUBLICATION_INVALID, binding->marker,
                "Static publication receiver has no accepted lexical symbol");
        }
        if (symbol->role == STATIC_PARAMETER || (symbol->role == STATIC_ALIAS && symbol->receiver == binding)) { continue; }
        StaticResultOutcome outcome;
        if (!static_result_evaluate_definition(plan->results, use->context, symbol, &outcome)) {
            (void)static_publication_dependencies(plan); return false;
        }
        StaticResultDefinitionUse *definition;
        if (!static_result_definition_use(plan->results, use->context, symbol, NULL, (SourceSpan){0}, &definition)) {
            (void)static_publication_dependencies(plan); return false;
        }
        if (!static_publication_definition(work, definition)) { return false; }
    }
    return true;
}
