/** @file
 * @brief Verify whole-graph root retention, exact witness identity and preparation failure ownership.
 */
#include "../common.c"
#include "semantic/static/results/graphs/internal.h"

/** This boundary needs only a named zero-input producer, not unrelated
 * ordinary-constructor or omitted-input fixture definitions.
 */
static const char *graph_target = "@...\n...out\n..integer\n";

/** Initialize actual source and registry owners without preparing graph work. */
static void setup_graphs(const char *caller, const char *target) {
    setup_guards(caller, target);
    initialize_results();
}

/** Require only an actual caller-provided creation context, never a guessed root. */
static const StaticResultContext *graph_context(const StaticTermEnvironment *environment) {
    const StaticCreationPath *path;
    const StaticResultContext *context;
    assert(static_creation_root(&guards, environment, &path));
    assert(static_result_context(&results, path, &context));
    return context;
}

/** A later statement receiving into a pair-declared name remains a separate
 * validation root even though it declares no new public symbol.
 */
static const char *graph_caller = "@...\n...known\n..integer\n.other\n ...out > ...received\n"
    ".other\n ...out > ...known\n";

/** Public definitions and equality-only statements are all queued independently;
 * graph preparation neither follows their RHS nor invents edges between them.
 */
static void it_queues_every_public_definition_and_statement_without_value_dependency_edges(void) {
    setup_graphs(graph_caller, graph_target);
    const StaticResultContext *context = graph_context(caller_environment);
    StaticResultGraph *graph, *again;
    assert(static_result_require_graph(&results, context, &graph));
    assert(graph->context == context && graph->work_count == 4 && !graph->local_complete);
    size_t definitions = 0, statements = 0;
    for (const StaticResultGraphWork *work = graph->first; work; work = work->next) {
        assert((work->definition != NULL) != (work->expression != NULL));
        if (work->definition) {
            definitions++;
            assert(work->definition->context == context && work->definition->path->parent == NULL);
            assert(work->definition->rhs == NULL && !work->definition->active);
        } else {
            statements++;
            assert(work->expression->context == context && work->expression->definition_path == NULL);
            assert(work->expression->producer == NULL && work->expression->requests == NULL);
        }
    }
    assert(definitions == 2 && statements == 2 && guards.definition_edge_count == 0 && guards.path_count == 1);
    assert(context->registry_use->candidate->local_started && !context->registry_use->candidate->local_complete);
    size_t before = allocation_count;
    assert(static_result_require_graph(&results, context, &again) && again == graph);
    assert(allocation_count == before && results.graph_count == 1);
    destroy_results();
}

/** Different actual roots of the same closed producer retain independent proof
 * opportunities; an already active key does not choose one path globally.
 */
static void it_retains_distinct_actual_root_witnesses_even_when_the_closed_key_is_shared(void) {
    setup_graphs(graph_caller, graph_target);
    const StaticResultContext *first = graph_context(caller_environment);
    StaticResultGraph *a, *b;
    assert(static_result_require_graph(&results, first, &a));
    const StaticTermEnvironment *canonical = first->registry_use->candidate->key;
    const StaticResultContext *second = graph_context(canonical);
    assert(first != second && first->registry_use->candidate == second->registry_use->candidate);
    assert(static_result_require_graph(&results, second, &b));
    assert(a != b && a->context == first && b->context == second && results.graph_count == 2);
    assert(result_registry.candidate_count == 1);
    assert(a->next == b && results.last_graph == b);
    destroy_results();
}

/** Ordinary contexts have no static graph; their later ordinary checking is
 * retained independently rather than fabricated as a graph or default instance.
 */
static void it_keeps_ordinary_contexts_outside_static_graph_validation(void) {
    setup_graphs("@!\nanchor&\n..integer\n", "@!\n");
    const StaticResultContext *context = graph_context(caller_environment);
    StaticResultGraph *output = NULL;
    size_t before = allocation_count;
    assert(static_result_require_graph(&results, context, &output));
    assert(output == NULL && results.graph_count == 0 && allocation_count == before);
    assert(context->registry_use->candidate->state == STATIC_CANDIDATE_NEW);
    destroy_results();
}

/** All source, guarded-root and canonical-equation preparation allocations can
 * fail before graph publication; partial work remains arena-owned.
 */
static void it_releases_every_partial_graph_preparation_after_allocation_refusal(void) {
    size_t total = 0;
    for (size_t failure = 0; failure <= total; failure++) {
        setup_graphs(graph_caller, graph_target);
        const StaticResultContext *context = graph_context(caller_environment);
        size_t before = allocation_count;
        if (failure) { failure_at = before + failure; }
        StaticResultGraph *output = NULL;
        bool result = static_result_require_graph(&results, context, &output);
        if (!failure) { assert(result && output); total = allocation_count - before; assert(total); }
        else {
            assert(!result && output == NULL && results.status == STATIC_RESULT_OUT_OF_MEMORY);
            assert(results.graph_count == 0 && results.first_graph == NULL && results.last_graph == NULL);
        }
        failure_at = 0;
        destroy_results();
    }
}

/** Unowned contexts, absent receivers and dense graph/work counters refuse
 * before publication; exact cached/partial records remain destructible.
 */
static void it_refuses_foreign_contexts_missing_receivers_and_graph_or_work_counter_overflow(void) {
    for (unsigned partition = 0; partition < 4; partition++) {
        setup_graphs(graph_caller, graph_target);
        const StaticResultContext *context = graph_context(caller_environment);
        StaticResultContext copied = *context;
        StaticResultGraph *output = NULL;
        if (partition == 2) { results.graph_count = SIZE_MAX; }
        bool result;
        if (partition == 3) {
            StaticResultUse *expression;
            assert(static_result_use(&results, context, result_projection(0), NULL, &expression));
            StaticResultGraph *partial = static_result_allocate(&results, sizeof(*partial));
            assert(partial);
            *partial = (StaticResultGraph){.context = context, .work_count = SIZE_MAX};
            result = static_result_graph_work(&results, partial, expression, NULL);
            assert(partial->first == NULL && partial->last == NULL && partial->work_count == SIZE_MAX);
        } else {
            result = static_result_require_graph(&results, partition == 0 ? &copied : context,
                partition == 1 ? NULL : &output);
        }
        assert(!result && output == NULL);
        assert(results.status == (partition >= 2 ? STATIC_RESULT_OUT_OF_MEMORY : STATIC_RESULT_INVALID));
        destroy_results();
    }
}

/** Execute exact local validation roots, actual contexts and ownership partitions. */
static void run(void) {
    it_queues_every_public_definition_and_statement_without_value_dependency_edges();
    it_retains_distinct_actual_root_witnesses_even_when_the_closed_key_is_shared();
    it_keeps_ordinary_contexts_outside_static_graph_validation();
    it_releases_every_partial_graph_preparation_after_allocation_refusal();
    it_refuses_foreign_contexts_missing_receivers_and_graph_or_work_counter_overflow();
}
/** Return after counted unconditional teardown of all borrowed owners. */
int main(void) { run(); return 0; }
