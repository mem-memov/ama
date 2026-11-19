/** @file
 * @brief Publish every actual static output in declaration order and only then its received aliases.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) validates unselected outputs and immutable names.
 */
#include "../internal.h"
#include "../../internal.h"

/** Already checked expression closure does not excuse a missing selected value;
 * query each declared output before exposing the enclosing publication.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) requires complete results.
 */
static bool output(StaticPublicationPlan *plan, const StaticTermEnvironment *environment,
    const StaticSymbol *symbol, StaticPublishedOutput **destination) {
    StaticConstraintSolver *solver = plan->results->guards->applications->solver;
    const StaticTerm *term, *value;
    StaticConstraintOutcome outcome;
    if (!static_term_symbol(solver->terms, environment, symbol, &term) ||
        !static_constraint_value(solver, term, &value, &outcome)) {
        (void)static_publication_dependencies(plan); return false;
    }
    if (outcome != STATIC_CONSTRAINT_COMPLETE) {
        return static_publication_fail(plan, STATIC_PUBLICATION_UNRESOLVED, symbol->site,
            "A required static output remains unresolved");
    }
    StaticPublishedOutput *item = static_publication_allocate(plan, sizeof(*item));
    if (item == NULL) { return false; }
    *item = (StaticPublishedOutput){.symbol = symbol, .value = value};
    *destination = item;
    return true;
}

/** Actual slot identities connect normalized receiver requests to closed output
 * values. Only named static receivers create alias descriptors.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves actual output order.
 */
bool static_publication_outputs(StaticPublicationPlan *plan, StaticPublication *publication) {
    const StaticResultUse *use = publication->use;
    const StaticResultContext *context = publication->context;
    if (!context) { return true; }
    const StaticTermEnvironment *environment = context->registry_use->candidate->key;
    const StaticModule *module = environment->definition;
    StaticPublishedOutput **tail = &publication->first_output;
    for (const StaticSymbol *symbol = module->first; symbol; symbol = symbol->next) {
        if (symbol->role != STATIC_RESULT && symbol->role != STATIC_ALIAS) { continue; }
        if (!output(plan, environment, symbol, tail)) { return false; }
        tail = &(*tail)->next;
        publication->output_count++;
    }
    if (!use) { return true; }
    const BindingApplication *requests = use->requests;
    for (const BindingRequest *request = requests->first; request; request = request->next) {
        if (request->direction != BINDING_STATIC_OUTPUT || request->omitted) { continue; }
        const StaticSymbol *symbol = static_publication_receiver(use, request->syntax.static_binding);
        if (symbol == NULL || request->slot == NULL) {
            return static_publication_fail(plan, STATIC_PUBLICATION_INVALID, request->span,
                "Closed output request has no accepted receiver or slot");
        }
        if (symbol->role != STATIC_ALIAS) { continue; }
        const StaticPublishedOutput *selected = publication->first_output;
        while (selected && selected->symbol != request->slot->static_symbol) { selected = selected->next; }
        if (selected == NULL) {
            return static_publication_fail(plan, STATIC_PUBLICATION_INVALID, request->span,
                "Closed output request selects no original producer output");
        }
        if (!static_publication_alias(plan, use->source->environment, symbol, selected->value)) { return false; }
    }
    return true;
}
