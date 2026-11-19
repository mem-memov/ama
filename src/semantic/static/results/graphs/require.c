/** @file
 * @brief Require actual static graph witnesses and close same-key ancestral recurrences without discarding independent paths.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) compares full keys before body expansion.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) checks every required producer graph.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns partial roots until teardown.
 */
#include <stdint.h>
#include "internal.h"

/** Only a required witness on this actual prefix can close recurrence without
 * new body work. A globally cached first path may have a stronger unresolved
 * ancestry and cannot suppress independent provenance replay.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) preserves order-independent pending work.
 */
static StaticResultGraph *ancestor_graph(StaticResultPlan *plan, const StaticResultContext *context,
    const StaticTermEnvironment *key) {
    for (const StaticCreationPath *path = context->creation->parent; path; path = path->parent) {
        if (!path->vertex || path->vertex->key != key) { continue; }
        for (StaticResultGraph *graph = plan->first_graph; graph; graph = graph->next) {
            if (graph->context->creation == path) { return graph; }
        }
    }
    return NULL;
}

/** New witness chains have no repeated expanded origin: unknown repetition
 * suspends in guards, a different key fails and an equal key links to an actual
 * ancestor witness. Finite initial contexts and finite source roots therefore
 * produce finite validation work, without a depth budget or invented root.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) bounds actual expansion.
 */
bool static_result_require_graph(StaticResultPlan *plan, const StaticResultContext *context,
    StaticResultGraph **output) {
    if (!static_result_dependencies(plan)) { return false; }
    if (!output || !static_result_has_context(plan, context)) {
        return static_result_fail(plan, STATIC_RESULT_INVALID, (SourceSpan){0}, "Invalid required static graph context or receiver");
    }
    if (context->creation->owner->environment->definition->definition->syntax.sections.kind != MODULE_STATIC) {
        *output = NULL;
        return true;
    }
    StaticRegistryKeyState state;
    if (!static_registry_scope_environment(plan->registry, context->registry_use, &state)) {
        (void)static_result_dependencies(plan); return false;
    }
    if (state == STATIC_REGISTRY_KEY_CLOSED && !context->registry_use->candidate->local_started) {
        bool claimed;
        if (!static_registry_begin_local(plan->registry, context->registry_use, &claimed, &state)) {
            (void)static_result_dependencies(plan); return false;
        }
    }
    for (StaticResultGraph *graph = plan->first_graph; graph; graph = graph->next) {
        if (graph->context == context) { *output = graph; return true; }
    }
    if (state == STATIC_REGISTRY_KEY_CLOSED) {
        StaticResultGraph *graph = ancestor_graph(plan, context, context->registry_use->candidate->key);
        if (graph) { *output = graph; return true; }
    }
    if (plan->graph_count == SIZE_MAX) {
        return static_result_fail(plan, STATIC_RESULT_OUT_OF_MEMORY, context->creation->site,
            "Required static graph count exceeds representable storage");
    }
    StaticResultGraph *graph = static_result_allocate(plan, sizeof(*graph));
    if (!graph) { return false; }
    graph->context = context;
    if (!static_result_graph_prepare(plan, graph) ||
        !static_registry_scope_environment(plan->registry, context->registry_use, &state)) {
        (void)static_result_dependencies(plan); return false;
    }
    StaticRegistryUse *registered;
    if (!static_registry_use(plan->registry, context->creation, &registered, &state)) {
        (void)static_result_dependencies(plan); return false;
    }
    if (plan->last_graph) { plan->last_graph->next = graph; }
    else { plan->first_graph = graph; }
    plan->last_graph = graph;
    plan->graph_count++;
    *output = graph;
    return true;
}
