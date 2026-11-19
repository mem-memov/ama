/** @file
 * @brief Deduplicate exact registered metadata proof handles before following their dependency graph.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) retains actual finite validation requirements.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns closure scratch.
 */
#include "internal.h"

/** Validate tagged identity before dereferencing a caller-supplied handle.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves exact scoped metadata and its validation obligations.
 */
static bool owned(StaticResultPlan *plan, StaticPublicationItem item) {
    if (item.kind == STATIC_CLOSURE_SOURCE) { return static_result_has_use(plan, item.as.source); }
    if (item.kind == STATIC_CLOSURE_DEFINITION) { return static_result_has_definition_use(plan, item.as.definition); }
    if (item.kind == STATIC_CLOSURE_GRAPH) {
        for (const StaticResultGraph *graph = plan->first_graph; graph; graph = graph->next) {
            if (graph == item.as.graph) { return true; }
        }
    }
    return false;
}

/** Compare only the active tagged pointer; never inactive union storage.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves exact scoped metadata and its validation obligations.
 */
static bool same(StaticPublicationItem a, StaticPublicationItem b) {
    if (a.kind != b.kind) { return false; }
    if (a.kind == STATIC_CLOSURE_SOURCE) { return a.as.source == b.as.source; }
    if (a.kind == STATIC_CLOSURE_DEFINITION) { return a.as.definition == b.as.definition; }
    return a.as.graph == b.as.graph;
}

/** A finite actual proof graph is traversed once per domain/handle. Linear
 * membership gives O(K^2) queue bookkeeping and O(K) attempt-owned storage.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) handles validation cycles without recursion.
 */
bool static_publication_enqueue(StaticPublicationWork *work, StaticPublicationItem item) {
    if (!owned(work->plan->results, item)) {
        return static_publication_fail(work->plan, STATIC_PUBLICATION_INVALID, (SourceSpan){0},
            "Static metadata closure has no registered proof handle");
    }
    for (const StaticPublicationWorkNode *node = work->first; node; node = node->next) {
        if (same(node->item, item)) { return true; }
    }
    StaticPublicationWorkNode *node = source_arena_allocate(&work->arena, sizeof(*node));
    if (!node) {
        return static_publication_fail(work->plan, STATIC_PUBLICATION_OUT_OF_MEMORY, (SourceSpan){0},
            "Static metadata closure could not allocate a proof work item");
    }
    *node = (StaticPublicationWorkNode){.item = item};
    if (work->last) { work->last->next = node; }
    else { work->first = node; }
    work->last = node;
    return true;
}
