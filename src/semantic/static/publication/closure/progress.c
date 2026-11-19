/** @file
 * @brief Observe real finite static facts and proof obligations instead of allocation or retry activity.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) permits only evidence-driven refinement.
 */
#include "progress.h"

/** Completed local graph flags are monotone until the owner fails; failures
 * abort refinement instead of being counted as another attempt.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12).
 */
StaticPublicationProgress static_publication_progress(const StaticPublicationPlan *plan) {
    const StaticConstraintSolver *solver = plan->results->guards->applications->solver;
    StaticPublicationProgress value = {.equations = solver->equation_count, .metadata = solver->metadata_count,
        .requirements = solver->requirement_count, .contexts = plan->results->context_count,
        .graphs = plan->results->graph_count, .caches = plan->results->cache_count};
    for (const StaticResultGraph *graph = plan->results->first_graph; graph; graph = graph->next) {
        value.complete_graphs += graph->local_complete;
    }
    return value;
}

/** Every changed component is a new structural fact, actual required context or
 * checked proof. Retrying the same request does not change these components.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12).
 */
bool static_publication_same_progress(StaticPublicationProgress a, StaticPublicationProgress b) {
    return a.equations == b.equations && a.metadata == b.metadata && a.requirements == b.requirements &&
        a.contexts == b.contexts && a.graphs == b.graphs && a.caches == b.caches &&
        a.complete_graphs == b.complete_graphs;
}
