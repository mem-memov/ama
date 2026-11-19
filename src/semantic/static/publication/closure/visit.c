/** @file
 * @brief Follow exact source, definition and graph proof requirements before metadata publication.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) retains complete producer validation through caches.
 */
#include "internal.h"

/** Source metadata may have no own value for a producer, but its full input map,
 * selected results and independently required graph must still close.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12).
 */
static bool source(StaticPublicationWork *work, const StaticResultUse *use) {
    work->closed &= use->outcome == STATIC_RESULT_READY && use->closed;
    if (use->required_graph && !static_publication_enqueue(work,
        (StaticPublicationItem){.kind = STATIC_CLOSURE_GRAPH, .as.graph = use->required_graph})) { return false; }
    for (const StaticResultDependency *edge = use->first_dependency; edge; edge = edge->next) {
        StaticPublicationItem item = edge->expression ?
            (StaticPublicationItem){.kind = STATIC_CLOSURE_SOURCE, .as.source = edge->expression} :
            (StaticPublicationItem){.kind = STATIC_CLOSURE_DEFINITION, .as.definition = edge->definition};
        if (!static_publication_enqueue(work, item)) { return false; }
    }
    return true;
}

/** Receiver evidence can close a local definition before its enclosing owner
 * input key is known. That fact is usable for inference, not publication of the
 * owner's whole producer. Keep the actual checked RHS and selected read proof.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) separates equalities and admission.
 */
static bool definition(StaticPublicationWork *work, const StaticResultDefinitionUse *use) {
    StaticConstraintOutcome outcome;
    const StaticTerm *value;
    if (!static_constraint_value(work->plan->results->guards->applications->solver,
        use->definition->value, &value, &outcome)) {
        (void)static_publication_dependencies(work->plan); return false;
    }
    work->closed &= outcome == STATIC_CONSTRAINT_COMPLETE && use->outcome == STATIC_RESULT_READY;
    const StaticResultUse *rhs = use->cache_hit ? use->checked_rhs : use->rhs;
    if (!rhs) { work->closed = false; return true; }
    if (!static_publication_enqueue(work,
        (StaticPublicationItem){.kind = STATIC_CLOSURE_SOURCE, .as.source = rhs})) { return false; }
    if (!use->cache_hit && use->selected && !static_publication_enqueue(work,
        (StaticPublicationItem){.kind = STATIC_CLOSURE_DEFINITION, .as.definition = use->selected})) { return false; }
    return true;
}

/** Full graph admission needs its own input key and every local source root.
 * Ordinary Type/body validation remains a separate retained registry obligation.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12).
 */
static bool graph(StaticPublicationWork *work, const StaticResultGraph *use) {
    StaticRegistryUse *registered;
    StaticRegistryKeyState keys;
    if (!static_registry_use(work->plan->results->registry, use->context->creation, &registered, &keys)) {
        (void)static_publication_dependencies(work->plan); return false;
    }
    work->closed &= keys == STATIC_REGISTRY_KEY_CLOSED && use->local_complete;
    for (const StaticResultGraphWork *root = use->first; root; root = root->next) {
        StaticPublicationItem item = root->expression ?
            (StaticPublicationItem){.kind = STATIC_CLOSURE_SOURCE, .as.source = root->expression} :
            (StaticPublicationItem){.kind = STATIC_CLOSURE_DEFINITION, .as.definition = root->definition};
        if (!static_publication_enqueue(work, item)) { return false; }
    }
    return true;
}

/** The enqueue boundary validated this exact tagged object before dispatch.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves exact scoped metadata and its validation obligations.
 */
bool static_publication_visit(StaticPublicationWork *work, StaticPublicationItem item) {
    if (item.kind == STATIC_CLOSURE_SOURCE) { return source(work, item.as.source); }
    if (item.kind == STATIC_CLOSURE_DEFINITION) { return definition(work, item.as.definition); }
    return graph(work, item.as.graph);
}
